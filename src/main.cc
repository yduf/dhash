/* compile with:
 *      g++ -g -Wall example.cc `pkg-config vips-cpp --cflags --libs`
 */

#include <cstdint>
#include <vips/vips8>


using namespace vips;
    int h = 11, w = 6;


VImage reduce( const char* name ) {
    VImage in = VImage::new_from_file (name,
                                       VImage::option ()
                                       //   ->set ("access", VIPS_ACCESS_SEQUENTIAL)
                                      );

    VImage reduced = in
                    .reducev( in.height()/ h)
                    .reduceh( in.width() / w)
                    .colourspace(VIPS_INTERPRETATION_sRGB, 
                                 VImage::option ()
                                    ->set ("source_space", VIPS_INTERPRETATION_B_W)
                                  )
                    ;

    return reduced;
}


int
main (int argc, char **argv)
{ 
  if (VIPS_INIT (argv[0])) 
    vips_error_exit (NULL);

  if (argc < 3+1)
    vips_error_exit ("usage: %s 0/1 w h file* ", argv[0]);

  int i = 1;
  int rot = atoi (argv[i++]);
  int width = atoi (argv[i++]);
  int height = atoi (argv[i++]);

  for( const char* name=argv[i++];i<=argc;name=argv[i++]) {

VImage reduced = reduce( name );
    reduced.write_to_file ("reduced.pgm");
    
    VImage A = reduced.crop( 0, 0, w - 1, h );
    VImage B = reduced.crop( 1, 0, w - 1, h );

    //A.write_to_file ("A.pgm");
    //B.write_to_file ("B.pgm");

    VImage diff = A < B; 
    diff.write_to_file ("diff.pgm");

    //printf ("%d %s\n", in.width (), name);
    uint8_t* p = (uint8_t*) diff.data();

    for( int i = 0; i < 11*5; ++i) {
        printf ("%d ", p[i]);
    }
    printf ("\n");


      for( int y = 0; y < 11; ++y) {
    for( int x = 0; x < 5; ++x) {
        std::vector<double> v = diff(x,y);
        printf ("%f ", v[0]);
      }
    printf ("\n");
    }
  }


  vips_shutdown ();

  return 0;
}