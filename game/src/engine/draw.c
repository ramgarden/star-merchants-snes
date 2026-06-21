#include "engine/draw.h"
#include "engine/console.h"
#include "engine/theme.h"

void smDrawHorizontalLine(u16 x,u16 y,u16 width){u16 i;for(i=0;i<width;i++)smPrint(x+i,y,SM_CHAR_HLINE);} 
void smDrawVerticalLine(u16 x,u16 y,u16 height){u16 i;for(i=0;i<height;i++)smPrint(x,y+i,SM_CHAR_VLINE);} 
void smDrawBox(u16 left,u16 top,u16 width,u16 height){u16 right,bottom;if(width<2||height<2)return;right=left+width-1;bottom=top+height-1;smDrawHorizontalLine(left+1,top,width-2);smDrawHorizontalLine(left+1,bottom,width-2);smDrawVerticalLine(left,top+1,height-2);smDrawVerticalLine(right,top+1,height-2);smPrint(left,top,SM_CHAR_TOP_LEFT);smPrint(right,top,SM_CHAR_TOP_RIGHT);smPrint(left,bottom,SM_CHAR_BOTTOM_LEFT);smPrint(right,bottom,SM_CHAR_BOTTOM_RIGHT);}
