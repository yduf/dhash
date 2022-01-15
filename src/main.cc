/* compile with:
 *      g++ -g -Wall example.cc `pkg-config vips-cpp --cflags --libs`
 */

#include <vips/vips8>

using namespace vips;

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

    VImage in = VImage::new_from_file (name,
                                        VImage::option ()->set ("access", VIPS_ACCESS_SEQUENTIAL))
                                        ;

    VImage reduced =  in.reducev( in.height()/ 21)
                    .reduceh( in.width() / 12)
                    .colourspace(VIPS_INTERPRETATION_sRGB, VImage::option ()->set ("source_space", VIPS_INTERPRETATION_B_W));


                    reduced.write_to_file ("reduced.ppm");
                    ;

    printf ("%d %s\n", in.width (), name);
  }


  vips_shutdown ();

  return 0;
}