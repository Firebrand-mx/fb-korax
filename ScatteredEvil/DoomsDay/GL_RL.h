#ifndef __OGL_REND_LIST_H__
#define __OGL_REND_LIST_H__


// ogl_rl.c

void RL_Init();
void RL_ClearLists();
void RL_DeleteLists();
void RL_AddPoly(rendpoly_t *poly);
void RL_PrepareFlat(rendpoly_t *poly, int numvrts, fvertex_t *origvrts, 
					int dir, subsector_t *subsector);
void RL_VertexColors(rendpoly_t *poly, int lightlevel);
void RL_RenderAllLists();

#endif