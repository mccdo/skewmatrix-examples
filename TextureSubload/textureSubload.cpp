// Name: OSG Texture Update
// Author: John Enright, Digital Transforms, for AlphaPixel.
// Creation Date: May 20, 2011

// Platform includes
#ifdef WIN32
#include <windows.h>
#endif

// STL includes
#include <string>
#include <iostream>

// OSG includes
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

// a convenience so don't have to prefix STL stuff with "std::".
using namespace std;

// --------------------------------------------------------------------------

// width & height of the test texture. It has to be large to really slow things
// down, and should be a power of 2 to prevent image scaling before moving to
// video ram.

#define IMAGE_WIDTH 8192
#define IMAGE_HEIGHT 8192

// width & height of the sub-image used to overlay on top of the texture.

#define SUB_IMG_WIDTH 500
#define SUB_IMG_HEIGHT 500

// --------------------------------------------------------------------------

void SetImageColor(osg::Image *img);
void SetImageColor(osg::Image *img, unsigned char red, unsigned char green, unsigned char blue);
void OverlayImage(osg::Image *dest, const osg::Image *subImg, int xOffset, int yOffset);

// --------------------------------------------------------------------------
// This is the necessary callback class for doing an image overlay directly
// onto a texture (aka subload).

class TextureSubloader : public osg::Texture2D::SubloadCallback
   {
   public:
      TextureSubloader() : xOffset(0), yOffset(0), doSubload(false) {}
      ~TextureSubloader() {}

      // create the OpenGL texture. A necessary override of osg::Texture2D::SubloadCallback (overrides a pure virtual).
      virtual void load(const osg::Texture2D &texture, osg::State &state) const
         {
         const osg::Image *image = texture.getImage();
         if (image)  // texture must have an image to work with.
            glTexImage2D(GL_TEXTURE_2D, 0, image->getPixelFormat(), image->s(), image->t(), 0, image->getPixelFormat(), image->getDataType(), image->data());
         }

      // overlay the image onto the texture. A necessary override of osg::Texture2D::SubloadCallback (overrides a pure virtual).
      virtual void subload(const osg::Texture2D &texture, osg::State &state) const
         {
         if ((doSubload == false) || (subImg.get() == 0))
            return;
         // copy the image into the current texture (the one currently bound / selected) at the given offsets.
         glEnable(GL_TEXTURE_2D);         // make sure 2D textures are enabled.
         glPixelStorei(GL_UNPACK_ALIGNMENT, subImg->getPacking());      // image unpacking.
         // it probably doesn't make sense to subload to a mipmap, but in any event, select the nearest one and operate on that.
         // in the case of a mipmapped texture, it is especially important to set these filters, or the result won't be shown.
         // a texture that doesn't have defined filters is treated as mipmapped.
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         // copy the image into place.
         glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, subImg->s(), subImg->t(), subImg->getPixelFormat(), subImg->getDataType(), subImg->data());
         doSubload = false;               // completed the subload. Need another call to DoImageUpdate() before the next one.
         subImg = 0;                      // don't hold on to the image pointer any longer.
         }

      // tell the next subload callback to copy the input image to the specified offsets
      // in the texture for this.
      void Update(osg::Image *img, int xOff, int yOff)
         {
         subImg = img;
         xOffset = xOff;
         yOffset = yOff;
         doSubload = true;
         }

   protected:
      int xOffset;                              // the X offset for the next subload operation.
      int yOffset;                              // the Y offset for the next subload operation.
      mutable bool doSubload;                   // true if should do a subload copy with next subload() callback for the texture.
      mutable osg::ref_ptr<osg::Image> subImg;  // a pointer to an image to overlay onto the texture for this.
   };

// **************************************************************************

int main(int argc, char **argv)

{
   // this will be true if using the slow texture update method (reload entire thing).
   // Run in slow mode to view the difference.
   bool useSlowMethod = false;

   // determine if the slow command line option was specified (-slow).
   if (argc > 1)
      {
      string opt = argv[1];
      while (opt.size() && (opt[0] == '-'))
         opt.erase(0, 1);
      if (opt.size())
         {
         if ((opt == "slow") || (opt == "SLOW") || (opt == "Slow") || (opt == "s") || (opt == "S"))
            useSlowMethod = true;
         }
      }

   // build a set of quad vertices.
   osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
   vertices->push_back(osg::Vec3(-0.5f, 0.0f, 0.5f));       // upper left corner
   vertices->push_back(osg::Vec3(-0.5f, 0.0f,-0.5f));       // bottom left corner
   vertices->push_back(osg::Vec3( 0.5f, 0.0f,-0.5f));       // bottom right corner
   vertices->push_back(osg::Vec3( 0.5f, 0.0f, 0.5f));       // upper right corner

   // set the normal
   osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
   normals->push_back(osg::Vec3(0.0f,-1.0f, 0.0f));

   // assign texture coordinates to match quad vertices.
   osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
   texcoords->push_back(osg::Vec2(0.0f, 1.0f));
   texcoords->push_back(osg::Vec2(0.0f, 0.0f));
   texcoords->push_back(osg::Vec2(1.0f, 0.0f));
   texcoords->push_back(osg::Vec2(1.0f, 1.0f));

   // create the OSG drawable, and add the vertices, normal, and texture coordinates.
   osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
   quad->setVertexArray( vertices.get() );
   quad->setNormalArray( normals.get() );
   quad->setNormalBinding( osg::Geometry::BIND_OVERALL );
   quad->setTexCoordArray( 0, texcoords.get() );
   quad->addPrimitiveSet( new osg::DrawArrays(GL_QUADS, 0, 4) );

   // create a fairly large image for use in a texture.
   osg::ref_ptr<osg::Image> image = new osg::Image;
   image->allocateImage(IMAGE_WIDTH, IMAGE_HEIGHT, 1, GL_RGB, GL_UNSIGNED_BYTE);
   if (image->data() == 0)
      {
      cout << "Insufficient memory to allocate an image of size " << IMAGE_WIDTH << " x " << IMAGE_HEIGHT << " pixels in GL_RGB format for use as a texture." << endl;
      return 10;
      }
   // fill the image with a color.
   SetImageColor(image.get(), 0, 0xff, 0);

   // create another image that will be used to copy onto the big texture image.
   osg::ref_ptr<osg::Image> subImg = new osg::Image;
   subImg->allocateImage(SUB_IMG_WIDTH, SUB_IMG_HEIGHT, 1, GL_RGB, GL_UNSIGNED_BYTE);
   if (subImg->data() == 0)
      {
      cout << "Insufficient memory to allocate an image of size " << SUB_IMG_WIDTH << " x " << SUB_IMG_HEIGHT << " pixels in GL_RGB format for use as a texture overlay." << endl;
      return 20;
      }

   // create a texture and give it the big image.
   osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
   texture->setImage(image.get());                 // set the texture image.
   osg::ref_ptr<TextureSubloader> subloader = 0;
   if (useSlowMethod == false)
      {
      // create a subloader object for fast sub-image texture overlays.
      subloader = new TextureSubloader;
      // attach the callback object to the texture.
      texture->setSubloadCallback(subloader.get());
      }

   // create an OSG Geode and add the drawable and texture to it.
   osg::ref_ptr<osg::Geode> root = new osg::Geode;
   root->addDrawable(quad.get());
   root->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());

   // view the scene.
   osgViewer::Viewer viewer;
   viewer.setSceneData(root.get());
   viewer.setCameraManipulator(new osgGA::TrackballManipulator);
   while (viewer.done() == false)
      {
      viewer.frame();
      // after each frame, fill the subimage with a new random color.
      SetImageColor( subImg.get() );
      // the slow way. Transfer and reload the entire texture from system memory to video memory.
      if (useSlowMethod)
         {
         OverlayImage(image.get(), subImg.get(), 250, 250);
         // mark the image as dirty, causing OSG to reload the entire texture from image during next frame.
         image->dirty();
         }
      else if (subloader)
         {
         // the fast way. update the texture data directly at the specified offsets during the next frame.
         subloader->Update(subImg.get(), 250, 250);
         }
      }
   return 0;
}

// **************************************************************************
// fill 'img' with a random color.

void SetImageColor(osg::Image *img)

{
   unsigned char red = (unsigned char)rand();
   unsigned char green = (unsigned char)rand();
   unsigned char blue = (unsigned char)rand();
   SetImageColor(img, red, green, blue);
}

// **************************************************************************
// fill 'img' with the supplied color.

void SetImageColor(osg::Image *img, unsigned char red, unsigned char green, unsigned char blue)

{
   int x, width = img->s();
   int y, height = img->t();
   unsigned int rowBytes = img->getRowSizeInBytes();
   unsigned char *rowData, *data = img->data();
   if (data == 0)
      return;
   for (y = 0; y < height; y++)
      {
      rowData = data;
      for (x = 0; x < width; x++)
         {
         *rowData++ = red;
         *rowData++ = green;
         *rowData++ = blue;
         }
      data += rowBytes;
      }
}

// **************************************************************************
// overlay 'subImg' onto 'dest' image at offsets xOffset, yOffset within dest.
//
// WARNING: no bounds checking or clipping is done here.

void OverlayImage(osg::Image *dest, const osg::Image *subImg, int xOffset, int yOffset)

{
   int y, height = subImg->t();
   unsigned int destRowBytes = dest->getRowSizeInBytes();
   unsigned int subImgRowBytes = subImg->getRowSizeInBytes();
   int xDataOffset = (dest->getPixelSizeInBits() / 8) * xOffset;
   unsigned char *destData = dest->data() + (yOffset * destRowBytes) + xDataOffset;
   const unsigned char *subImgData = subImg->data();
   if ((destData == 0) || (subImgData == 0))
      return;
   // copy each row of subImg into position inside of dest.
   for (y = 0; y < height; y++)
      {
      // copy an entire row of subImg.
      memcpy(destData, subImgData, subImgRowBytes);
      // update the memory pointers (pointers to the raw pixel data).
      subImgData += subImgRowBytes;
      destData += destRowBytes;
      }
}
