/* visset - Visualize isl sets
 * Sven van Haastregt
 * LIACS, Leiden University
 *
 * Example usage:
 *   echo "{[i,j] : j >= 2 and j <= 6 and i >= 10 and i <= 20}" | ./visset > out.gnu
 * Then open gnuplot and run:
 *   load "out.gnu"
 */
#include <isl/ctx.h>
#include <isl/set.h>

// isl callback to print a point
int print_point(__isl_take isl_point *pnt, void *user) {
  FILE *out = (FILE*)user;
  isl_int value;
  isl_int_init(value);
  isl_space* space = isl_point_get_space(pnt);
  int dimSize = isl_space_dim(space, isl_dim_set);
  isl_space_free(space);

  // Print coordinate in each dimension, whitespace-separated.
  for (int i = 0; i < dimSize; i++) {
    isl_point_get_coordinate(pnt, isl_dim_set, i, &value);
    isl_int_print(out, value, 5);
  }
  fprintf(out, "\n");

  isl_int_clear(value);
  isl_point_free(pnt);
  return 0;
}


int main() {
  FILE *out = stdout;
  isl_ctx *ctx = isl_ctx_alloc();

  isl_set *set = isl_set_read_from_file(ctx, stdin);
  if (!set) {
    fprintf(stderr, "The input is not a valid isl set.\n");
    exit(1);
  }

  int dim = isl_set_dim(set, isl_dim_set);
  if (dim != 2 && dim != 3) {
    fprintf(stderr, "Only 2D and 3D sets are currently supported!\n");
    exit(1);
  }

  // Gnuplot setup
  fprintf(out, "# gnuplot script\n");
  fprintf(out, "# To render, use:\n");
  fprintf(out, "# load \"thisfile\"\n");
  fprintf(out, "\n");
  fprintf(out, "set terminal X11\n");
  fprintf(out, "set noborder\n");
  fprintf(out, "set tics\n");
  fprintf(out, "unset key\n");
  fprintf(out, "set pointsize 2.0\n");
  fprintf(out, "set xlabel \"%s\"\n", isl_set_get_dim_name(set, isl_dim_set, 0));
  fprintf(out, "set ylabel \"%s\"\n", isl_set_get_dim_name(set, isl_dim_set, 1));
  if (dim == 3)
    fprintf(out, "set zlabel \"%s\"\n", isl_set_get_dim_name(set, isl_dim_set, 2));

  // The actual plot command
  fprintf(out, "%splot '-' pointtype 7 \\\n", dim==3 ? "s" : "");
  fprintf(out, "\n");

  // List of points
  isl_set_foreach_point(set, print_point, (void*)out);
  fprintf(out, "e\n");

  isl_set_free(set);
  isl_ctx_free(ctx);
  return 0;
}
