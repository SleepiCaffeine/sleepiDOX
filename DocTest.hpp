/* @sleepiDOX This function is a template function
 Practically does nothing, except sum the two params and return the sum
 doo doo
*/
int sum(int a, int b);

#define Vector2i int // Just for the example to work
#define CHART template <typename CharT>

/* @sleepiDOX Function modifies given vector, and normalizes it
Returns nothing, all operations are done in-place */

// @sleepiPARAM vec : vector to normalize

// @sleepiPARAM scale : optionally scales vector
// @sleepiRETURNS nothing



CHART void normalize_vector(Vector2i& vec, const int scale = 1);