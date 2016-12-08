/*
 * color.h - Data structures and function prototypes for coloring algorithm
 *             to determine register allocation.
 */
#include "liveness.h"

struct COL_result {Temp_map coloring; Temp_tempList spills;};
struct COL_result COL_color(struct Live_graph lg, Temp_map initial, Temp_tempList regs);


