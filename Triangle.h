/*****************************************************************************/
/*                                                                           */
/*  (triangle.h)                                                             */
/*                                                                           */
/*  Include file for programs that call Triangle.                            */
/*                                                                           */
/*  Accompanies Triangle Version 1.6                                         */
/*  July 28, 2005                                                            */
/*                                                                           */
/*  Copyright 1996, 2005                                                     */
/*  Jonathan Richard Shewchuk                                                */
/*  2360 Woolsey #H                                                          */
/*  Berkeley, California  94705-1927                                         */
/*  jrs@cs.berkeley.edu                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  How to call Triangle from another program                                */
/*                                                                           */
/*                                                                           */
/*  If you haven't read Triangle's instructions (run "triangle -h" to read   */
/*  them), you won't understand what follows.                                */
/*                                                                           */
/*  Triangle must be compiled into an object file (triangle.o) with the      */
/*  TRILIBRARY symbol defined (generally by using the -DTRILIBRARY compiler  */
/*  switch).  The makefile included with Triangle will do this for you if    */
/*  you run "make trilibrary".  The resulting object file can be called via  */
/*  the procedure triangulate().                                             */
/*                                                                           */
/*  If the size of the object file is important to you, you may wish to      */
/*  generate a reduced version of triangle.o.  The REDUCED symbol gets rid   */
/*  of all features that are primarily of research interest.  Specifically,  */
/*  the -DREDUCED switch eliminates Triangle's -i, -F, -s, and -C switches.  */
/*  The CDT_ONLY symbol gets rid of all meshing algorithms above and beyond  */
/*  constrained Delaunay triangulation.  Specifically, the -DCDT_ONLY switch */
/*  eliminates Triangle's -r, -q, -a, -u, -D, -Y, -S, and -s switches.       */
/*                                                                           */
/*  IMPORTANT:  These definitions (TRILIBRARY, REDUCED, CDT_ONLY) must be    */
/*  made in the makefile or in triangle.c itself.  Putting these definitions */
/*  in this file (triangle.h) will not create the desired effect.            */
/*                                                                           */
/*                                                                           */
/*  The calling convention for triangulate() follows.                        */
/*                                                                           */
/*      void triangulate(triswitches, in, out, vorout)                       */
/*      char *triswitches;                                                   */
/*      struct triangulateio *in;                                            */
/*      struct triangulateio *out;                                           */
/*      struct triangulateio *vorout;                                        */
/*                                                                           */
/*  `triswitches' is a string containing the command line switches you wish  */
/*  to invoke.  No initial dash is required.  Some suggestions:              */
/*                                                                           */
/*  - You'll probably find it convenient to use the `z' switch so that       */
/*    points (and other items) are numbered from zero.  This simplifies      */
/*    indexing, because the first item of any type always starts at index    */
/*    [0] of the corresponding array, whether that item's number is zero or  */
/*    one.                                                                   */
/*  - You'll probably want to use the `Q' (quiet) switch in your final code, */
/*    but you can take advantage of Triangle's printed output (including the */
/*    `V' switch) while debugging.                                           */
/*  - If you are not using the `q', `a', `u', `D', `j', or `s' switches,     */
/*    then the output points will be identical to the input points, except   */
/*    possibly for the boundary markers.  If you don't need the boundary     */
/*    markers, you should use the `N' (no nodes output) switch to save       */
/*    memory.  (If you do need boundary markers, but need to save memory, a  */
/*    good nasty trick is to set out->pointList equal to in->pointList       */
/*    before calling triangulate(), so that Triangle overwrites the input    */
/*    points with identical copies.)                                         */
/*  - The `I' (no iteration numbers) and `g' (.off file output) switches     */
/*    have no effect when Triangle is compiled with TRILIBRARY defined.      */
/*                                                                           */
/*  `in', `out', and `vorout' are descriptions of the input, the output,     */
/*  and the Voronoi output.  If the `v' (Voronoi output) switch is not used, */
/*  `vorout' may be NULL.  `in' and `out' may never be NULL.                 */
/*                                                                           */
/*  Certain fields of the input and output structures must be initialized,   */
/*  as described below.                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  The `triangulateio' structure.                                           */
/*                                                                           */
/*  Used to pass data into and out of the triangulate() procedure.           */
/*                                                                           */
/*                                                                           */
/*  Arrays are used to store points, triangles, markers, and so forth.  In   */
/*  all cases, the first item in any array is stored starting at index [0].  */
/*  However, that item is item number `1' unless the `z' switch is used, in  */
/*  which case it is item number `0'.  Hence, you may find it easier to      */
/*  index points (and triangles in the neighbor list) if you use the `z'     */
/*  switch.  Unless, of course, you're calling Triangle from a Fortran       */
/*  program.                                                                 */
/*                                                                           */
/*  Description of fields (except the `numberof' fields, which are obvious): */
/*                                                                           */
/*  `pointList':  An array of point coordinates.  The first point's x        */
/*    coordinate is at index [0] and its y coordinate at index [1], followed */
/*    by the coordinates of the remaining points.  Each point occupies two   */
/*    REALs.                                                                 */
/*  `pointAttrList':  An array of point attributes.  Each point's       */
/*    attributes occupy `numOfPointAttrs' REALs.                     */
/*  `pointMarkList':  An array of point markers; one int per point.        */
/*                                                                           */
/*  `triList':  An array of triangle corners.  The first triangle's     */
/*    first corner is at index [0], followed by its other two corners in     */
/*    counterclockwise order, followed by any other nodes if the triangle    */
/*    represents a nonlinear element.  Each triangle occupies                */
/*    `numOfCorners' ints.                                                */
/*  `triAttrList':  An array of triangle attributes.  Each         */
/*    triangle's attributes occupy `numOfTriAttributes' REALs.       */
/*  `triAreaList':  An array of triangle area constraints; one REAL per */
/*    triangle.  Input only.                                                 */
/*  `neighborList':  An array of triangle neighbors; three ints per          */
/*    triangle.  Output only.                                                */
/*                                                                           */
/*  `segmentList':  An array of segment endpoints.  The first segment's      */
/*    endpoints are at indices [0] and [1], followed by the remaining        */
/*    segments.  Two ints per segment.                                       */
/*  `segMarkList':  An array of segment markers; one int per segment.  */
/*                                                                           */
/*  `holeList':  An array of holes.  The first hole's x and y coordinates    */
/*    are at indices [0] and [1], followed by the remaining holes.  Two      */
/*    REALs per hole.  Input only, although the pointer is copied to the     */
/*    output structure for your convenience.                                 */
/*                                                                           */
/*  `regionList':  An array of regional attributes and area constraints.     */
/*    The first constraint's x and y coordinates are at indices [0] and [1], */
/*    followed by the regional attribute at index [2], followed by the       */
/*    maximum area at index [3], followed by the remaining area constraints. */
/*    Four REALs per area constraint.  Note that each regional attribute is  */
/*    used only if you select the `A' switch, and each area constraint is    */
/*    used only if you select the `a' switch (with no number following), but */
/*    omitting one of these switches does not change the memory layout.      */
/*    Input only, although the pointer is copied to the output structure for */
/*    your convenience.                                                      */
/*                                                                           */
/*  `edgeList':  An array of edge endpoints.  The first edge's endpoints are */
/*    at indices [0] and [1], followed by the remaining edges.  Two ints per */
/*    edge.  Output only.                                                    */
/*  `edgeMarkList':  An array of edge markers; one int per edge.  Output   */
/*    only.                                                                  */
/*  `normList':  An array of normal vectors, used for infinite rays in       */
/*    Voronoi diagrams.  The first normal vector's x and y magnitudes are    */
/*    at indices [0] and [1], followed by the remaining vectors.  For each   */
/*    finite edge in a Voronoi diagram, the normal vector written is the     */
/*    zero vector.  Two REALs per edge.  Output only.                        */
/*                                                                           */
/*                                                                           */
/*  Any input fields that Triangle will examine must be initialized.         */
/*  Furthermore, for each output array that Triangle will write to, you      */
/*  must either provide space by setting the appropriate pointer to point    */
/*  to the space you want the data written to, or you must initialize the    */
/*  pointer to NULL, which tells Triangle to allocate space for the results. */
/*  The latter option is preferable, because Triangle always knows exactly   */
/*  how much space to allocate.  The former option is provided mainly for    */
/*  people who need to call Triangle from Fortran code, though it also makes */
/*  possible some nasty space-saving tricks, like writing the output to the  */
/*  same arrays as the input.                                                */
/*                                                                           */
/*  Triangle will not free() any input or output arrays, including those it  */
/*  allocates itself; that's up to you.  You should free arrays allocated by */
/*  Triangle by calling the trifree() procedure defined below.  (By default, */
/*  trifree() just calls the standard free() library procedure, but          */
/*  applications that call triangulate() may replace trimalloc() and         */
/*  trifree() in triangle.c to use specialized memory allocators.)           */
/*                                                                           */
/*  Here's a guide to help you decide which fields you must initialize       */
/*  before you call triangulate().                                           */
/*                                                                           */
/*  `in':                                                                    */
/*                                                                           */
/*    - `pointList' must always point to a list of points; `numOfPoints'  */
/*      and `numOfPointAttrs' must be properly set.                  */
/*      `pointMarkList' must either be set to NULL (in which case all      */
/*      markers default to zero), or must point to a list of markers.  If    */
/*      `numOfPointAttrs' is not zero, `pointAttrList' must     */
/*      point to a list of point attributes.                                 */
/*    - If the `r' switch is used, `triList' must point to a list of    */
/*      triangles, and `numOfTriangles', `numOfCorners', and           */
/*      `numOfTriAttributes' must be properly set.  If               */
/*      `numOfTriAttributes' is not zero, `triAttrList'    */
/*      must point to a list of triangle attributes.  If the `a' switch is   */
/*      used (with no number following), `triAreaList' must point to a  */
/*      list of triangle area constraints.  `neighborList' may be ignored.   */
/*    - If the `p' switch is used, `segmentList' must point to a list of     */
/*      segments, `numOfSegments' must be properly set, and               */
/*      `segMarkList' must either be set to NULL (in which case all    */
/*      markers default to zero), or must point to a list of markers.        */
/*    - If the `p' switch is used without the `r' switch, then               */
/*      `numOfHoles' and `numOfRegions' must be properly set.  If      */
/*      `numOfHoles' is not zero, `holeList' must point to a list of      */
/*      holes.  If `numOfRegions' is not zero, `regionList' must point to */
/*      a list of region constraints.                                        */
/*    - If the `p' switch is used, `holeList', `numOfHoles',              */
/*      `regionList', and `numOfRegions' is copied to `out'.  (You can    */
/*      nonetheless get away with not initializing them if the `r' switch is */
/*      used.)                                                               */
/*    - `edgeList', `edgeMarkList', `normList', and `numOfEdges' may be */
/*      ignored.                                                             */
/*                                                                           */
/*  `out':                                                                   */
/*                                                                           */
/*    - `pointList' must be initialized (NULL or pointing to memory) unless  */
/*      the `N' switch is used.  `pointMarkList' must be initialized       */
/*      unless the `N' or `B' switch is used.  If `N' is not used and        */
/*      `in->numOfPointAttrs' is not zero, `pointAttrList' must */
/*      be initialized.                                                      */
/*    - `triList' must be initialized unless the `E' switch is used.    */
/*      `neighborList' must be initialized if the `n' switch is used.  If    */
/*      the `E' switch is not used and (`in->numberofelementattributes' is   */
/*      not zero or the `A' switch is used), `elementattributelist' must be  */
/*      initialized.  `triAreaList' may be ignored.                     */
/*    - `segmentList' must be initialized if the `p' or `c' switch is used,  */
/*      and the `P' switch is not used.  `segMarkList' must also be    */
/*      initialized under these circumstances unless the `B' switch is used. */
/*    - `edgeList' must be initialized if the `e' switch is used.            */
/*      `edgeMarkList' must be initialized if the `e' switch is used and   */
/*      the `B' switch is not.                                               */
/*    - `holeList', `regionList', `normList', and all scalars may be ignored.*/
/*                                                                           */
/*  `vorout' (only needed if `v' switch is used):                            */
/*                                                                           */
/*    - `pointList' must be initialized.  If `in->numOfPointAttrs'   */
/*      is not zero, `pointAttrList' must be initialized.               */
/*      `pointMarkList' may be ignored.                                    */
/*    - `edgeList' and `normList' must both be initialized.                  */
/*      `edgeMarkList' may be ignored.                                     */
/*    - Everything else may be ignored.                                      */
/*                                                                           */
/*  After a call to triangulate(), the valid fields of `out' and `vorout'    */
/*  will depend, in an obvious way, on the choice of switches used.  Note    */
/*  that when the `p' switch is used, the pointers `holeList' and            */
/*  `regionList' are copied from `in' to `out', but no new space is          */
/*  allocated; be careful that you don't free() the same array twice.  On    */
/*  the other hand, Triangle will never copy the `pointList' pointer (or any */
/*  others); new space is allocated for `out->pointList', or if the `N'      */
/*  switch is used, `out->pointList' remains uninitialized.                  */
/*                                                                           */
/*  All of the meaningful `numberof' fields will be properly set; for        */
/*  instance, `numOfEdges' will represent the number of edges in the      */
/*  triangulation whether or not the edges were written.  If segments are    */
/*  not used, `numOfSegments' will indicate the number of boundary edges. */
/*                                                                           */
/*****************************************************************************/
#ifndef TRIANGLE.H
#define TRIANGLE.H

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */

struct triangulateio {
  REAL *pointList;                                               /* In / out */
  REAL *pointAttrList;                                      /* In / out */
  int *pointMarkList;                                          /* In / out */
  int numOfPoints;                                            /* In / out */
  int numOfPointAttrs;                                   /* In / out */

  int *triList;                                             /* In / out */	//  Point list of a triangle. [ZuoW,2010/3/8]
  REAL *triAttrList;                                   /* In / out */
  REAL *triAreaList;                                         /* In only */
  int *neighborList;                                             /* Out only */
  int numOfTriangles;                                         /* In / out */
  int numOfCorners;                                           /* In / out */
  int numOfTriAttributes;                                /* In / out */

  int *segmentList;                                              /* In / out */
  int *segMarkList;                                        /* In / out */
  int numOfSegments;                                          /* In / out */

  REAL *holeList;                        /* In / pointer to array copied out */
  int numOfHoles;                                      /* In / copied out */

  REAL *regionList;                      /* In / pointer to array copied out */
  int numOfRegions;                                    /* In / copied out */

  int *edgeList;                                                 /* Out only */
  int *edgeMarkList;            /* Not used with Voronoi diagram; out only */
  REAL *normList;                /* Used only with Voronoi diagram; out only */
  int numOfEdges;                                             /* Out only */
};

#define ANSI_DECLARATORS
#ifdef ANSI_DECLARATORS
void triangulate(char *, struct triangulateio *, struct triangulateio *,
                 struct triangulateio *);
void trifree(void *memptr);
#else /* not ANSI_DECLARATORS */
void triangulate();
void trifree();
#endif /* not ANSI_DECLARATORS */

#endif	// TRIANGLE.H
