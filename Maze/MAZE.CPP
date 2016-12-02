#include <graphics.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <bios.h>
#include <dos.h>

#define MAZE_MAX_WIDTH  48
#define MAZE_MAX_HEIGHT 36
#define MAZE_TOP        24
#define MAZE_CENTER    320
#define MAZE_CELL_SIZE  12
#define LEFT (639-mw*MAZE_CELL_SIZE)/2;
#define TOP	 (479-mh*MAZE_CELL_SIZE)/2+2;
#define BGCOLOR     BLACK
#define MAZECOLOR   WHITE
#define PATHCOLOR  YELLOW
#define TRACECOLOR  GREEN
#define BADCOLOR      RED

#define ESC      283
#define F1     15104
#define F2     15360
#define F3     15616
#define HOME   18176
#define END    20224
#define PAGEUP 18688
#define PGDOWN 20736
#define LEFTKEY  19200
#define UPKEY    18432
#define RIGHTKEY 19712
#define DOWNKEY  20480

char caption[]="Maze 2002  http://myafx.yeah.net";
char prompt1[]="F1=Show Path  F2=New Maze  F3=Redraw Maze  ArrowKeys=Move  Esc=Exit";
char prompt2[]="Home/End=Change Width  PageUp/PageDown=Change Height  ";

enum direction {west,north,east,south};

class Point
{
public:
	int x,y;
	Point();
	~Point(){}
	Point(int,int);
	Point & operator=(const Point &);
};
Point::Point()
{
	x=y=0;
}
Point::Point(int ax,int ay)
{
	x=ax;
	y=ay;
}
Point & Point::operator=(const Point &p)
{
	x=p.x;
	y=p.y;
	return *this;
}

class TwoDimensionArray
{
private:
	int width,height;
	int **pM,*spM;
	void Clear();
public:
	TwoDimensionArray();
	TwoDimensionArray(int,int);
	~TwoDimensionArray();
	int SetSize(int,int);
	int Get(int,int) const ;
	int Set(int,int,int);
	int GetWidth() const {return width;}
	int GetHeight() const {return height;}
};
TwoDimensionArray::TwoDimensionArray()
{
	width=height=0;
	spM=NULL;
	pM=NULL;
}
TwoDimensionArray::~TwoDimensionArray()
{
	Clear();
}
int TwoDimensionArray::SetSize(int w,int h)
{
	int i,j;
	Clear();
	width=w;
	height=h;
	spM=new int[w*h];
	if (spM==NULL)
	{
		printf("Not enough memory!\n");
		exit(1);
		return 0;
	}
	pM=new int* [w];
	if (pM==NULL)
	{
		delete []spM;
		printf("Not enough memory!\n");
		exit(1);
		return 0;
	}
	for (j=0;j<w;j++)
		pM[j]=spM+j*h;
	for (i=0;i<w;i++)
		for (j=0;j<h;j++)
			pM[i][j]=0;

	return 1;
}
TwoDimensionArray::TwoDimensionArray(int w,int h):spM(NULL),pM(NULL)
{
	SetSize(w,h);
}
void TwoDimensionArray::Clear()
{
	if (pM!=NULL)
		delete []pM;
	if (spM!=NULL)
		delete []spM;
	width=height=0;
}
int TwoDimensionArray::Get (int x,int y) const
{
	return pM[x][y];
}
int TwoDimensionArray::Set(int x,int y,int v)
{
	pM[x][y]=v;
	return v;
}


class Graph
{
public:
	Graph();
	~Graph();
};
Graph G;
Graph::Graph()
{
	int gdriver = VGA, gmode=VGAHI, errorcode;

	errorcode = registerbgidriver(EGAVGA_driver);

	/* report any registration errors */
	if (errorcode < 0)
	{
		printf("Graphics error: %s\n", grapherrormsg(errorcode));
		printf("Press any key to halt:");
		getch();
		exit(1); /* terminate with an error code */
	}
	errorcode = registerbgifont(triplex_font);
	/* report any registration errors */
	if (errorcode < 0)
	{
		printf("Graphics error: %s\n", grapherrormsg(errorcode));
		printf("Press any key to halt:");
		getch();
		exit(1); /* terminate with an error code */
	}


   /* initialize graphics mode */
   initgraph(&gdriver, &gmode, "e:\\tc\\bgi");

   /* read result of initialization */
   errorcode = graphresult();

   if (errorcode != grOk)  /* an error occurred */
   {
	  printf("Graphics error: %s\n", grapherrormsg(errorcode));
	  printf("Press any key to halt:");
	  getch();
	  exit(1);             /* return with error code */
   }

}
Graph::~Graph()
{
	closegraph();
}

class Maze
{
private:
	int width,height;
	Point start,end;
	TwoDimensionArray maze,path;
	int ExtendableInDir(const TwoDimensionArray &,const Point &,direction);
	int Extendable(const TwoDimensionArray &,const Point &);
	Point FindExtendablePosition(const TwoDimensionArray &);
	direction RandDirection(int,int,int,int);
	void GenerateMaze();
	void GeneratePath();
	void DetectPath(TwoDimensionArray &,Point &,int &);

public:
	Maze();
	Maze(int,int);
	int SetSize(int,int);
	void SetStart(const Point &);
	void SetEnd(const Point &);
	const Point & GetStart() const;
	const Point & GetEnd() const;

	int GetMaze(int,int) const;
	int GetPath(int,int) const;
	int GetWidth() const{return width;}
	int GetHeight()const{return height;}
};
Maze::Maze()
{
	width=height=0;
}
int Maze::GetMaze(int x,int y) const
{
	return maze.Get(x,y);
}
int Maze::GetPath(int x,int y) const
{
	return path.Get(x,y);
}
int Maze::SetSize(int w,int h)
{
	int i;
	if (w<=1||h<=1)
	{
		printf("Meanless size!");
		return 0;
	}
	if (w>MAZE_MAX_WIDTH)
	{
		printf("Width is too large!");
		return 0;
	}
	if (h>MAZE_MAX_HEIGHT)
	{
		printf("Height is too large!");
		return 0;
	}
	width=w;
	height=h;
	start.x=0;
	start.y=0;  /* entrance of the maze is at upper left corner */
	end.x=w-1;
	end.y=h-1;  /* exit of the maze is at lower right corner */
	maze.SetSize(w,h);
	path.SetSize(w,h);

	GenerateMaze();
	GeneratePath();


	return 1;
}
Maze::Maze(int w,int h)
{
	SetSize(w,h);
}

void Maze::SetStart(const Point &p)
{
	start=p;
}
void Maze::SetEnd(const Point &p)
{
	end=p;
}
const Point & Maze::GetStart() const
{
	return start;
}
const Point & Maze::GetEnd() const
{
	return end;
}

int Maze::ExtendableInDir(const TwoDimensionArray & ta,const Point & p,direction dir)
{
	switch (dir)
	{
	case west:
		if (p.x>0 && (ta.Get(p.x-1,p.y)==0))
			return 1;
		return 0;
	case north:
		if (p.y>0 && (ta.Get(p.x,p.y-1)==0))
			return 1;
		return 0;
	case east:
		if (p.x<(ta.GetWidth()-1) && (ta.Get(p.x+1,p.y)==0))
			return 1;
		return 0;
	default: /* south */
		if (p.y<(ta.GetHeight()-1) && (ta.Get(p.x,p.y+1)==0))
			return 1;
	}
	return 0;
}

int Maze::Extendable(const TwoDimensionArray & ta,const Point & p)
{
	if ( ((ExtendableInDir(ta,p, west)) ||
		(ExtendableInDir(ta,p,north)) ||
		(ExtendableInDir(ta,p, east)) ||
		(ExtendableInDir(ta,p,south))) && ta.Get(p.x,p.y) )
		return 1;
	return 0;
}
Point Maze::FindExtendablePosition(const TwoDimensionArray & ta)
{
	int i,j,w,h,select;
	Point *tp,rp;
	int count=0;

	w=ta.GetWidth();
	h=ta.GetHeight();

	tp=new Point[w*h];

	for (i=0;i<w;i++)
		for (j=0;j<h;j++)
		{
			if ( Extendable(ta,Point(i,j)) )
			{
				tp[count].x=i;
				tp[count].y=j;
				count++;
			}
		}
	if (count==0)
		rp.x=rp.y=-1;
	else
	{
		select=rand()%count;
		rp=tp[select];
	}
	delete []tp;

	return rp;
}

direction Maze::RandDirection(int w,int n,int e,int s)
{
	direction dir[4];
	int count=0,select;
	if (e)
	{
		dir[count]=east;
		count++;
	}
	if (w)
	{
		dir[count]=west;
		count++;
	}
	if (s)
	{
		dir[count]=south;
		count++;
	}
	if (n)
	{
		dir[count]=north;
		count++;
	}


	select=rand()%count;
	return dir[select];
}

void Maze::GenerateMaze()
{
	int w,n,e,s;
	direction dir;
	Point pos(rand()%width,rand()%height);
	TwoDimensionArray temp(width,height);
	temp.Set(pos.x,pos.y,1); /* mark the position */
	for (;;)
	{
		pos=FindExtendablePosition(temp);

		if (pos.x==-1&&pos.y==-1) break; /* constructing maze complete */
		for (;;)
		{

			w=ExtendableInDir(temp,pos, west);
			n=ExtendableInDir(temp,pos,north);
			e=ExtendableInDir(temp,pos, east);
			s=ExtendableInDir(temp,pos,south);
			dir=RandDirection(w,n,e,s);

			switch(dir)
			{
			case  west:
				pos.x-=1;
				maze.Set(pos.x,pos.y,2);
				break;
			case north:
				pos.y-=1;
				maze.Set(pos.x,pos.y,1);
				break;
			case  east:
				if (maze.Get(pos.x,pos.y)==0)
					maze.Set(pos.x,pos.y,2);
				else
					maze.Set(pos.x,pos.y,3);
					pos.x+=1;
				break;
			case south:

				if (maze.Get(pos.x,pos.y)==0)
					maze.Set(pos.x,pos.y,1);
				else
					maze.Set(pos.x,pos.y,3);
				pos.y+=1;
			}
			temp.Set(pos.x,pos.y,1); /* mark the position */
			if (!Extendable(temp,pos))
				break;
		}
	}
}

void Maze::GeneratePath()
{
	Point cur=start;
	int success=0;
	path.Set(cur.x,cur.y,1); /* mark start position */
	DetectPath(path,cur,success);

}
void Maze::DetectPath(TwoDimensionArray & ta,Point & cur,int & success)
{
	if (cur.x==end.x&&cur.y==end.y)
	{
		success=1;
		return;
	}
	if (cur.x>0 && (maze.Get(cur.x-1,cur.y) & 0x2) && (ta.Get(cur.x-1,cur.y)==0)) /* west */
	{
		cur.x-=1;
		ta.Set(cur.x,cur.y,1);
		DetectPath(ta,cur,success);
		if (success) return;
		ta.Set(cur.x,cur.y,0);
		cur.x+=1;
	}
	if (cur.y>0 && (maze.Get(cur.x,cur.y-1) & 0x1) && (ta.Get(cur.x,cur.y-1)==0)) /* north */
	{
		cur.y-=1;
		ta.Set(cur.x,cur.y,1);
		DetectPath(ta,cur,success);
		if (success) return;
		ta.Set(cur.x,cur.y,0);
		cur.y+=1;
	}
	if ((maze.Get(cur.x,cur.y) & 0x2) && (ta.Get(cur.x+1,cur.y)==0)) /* east */
	{
		cur.x+=1;
		ta.Set(cur.x,cur.y,1);
		DetectPath(ta,cur,success);
		if (success) return;
		ta.Set(cur.x,cur.y,0);
		cur.x-=1;
	}
	if ((maze.Get(cur.x,cur.y) & 0x1) && (ta.Get(cur.x,cur.y+1)==0)) /* south */
	{
		cur.y+=1;
		ta.Set(cur.x,cur.y,1);
		DetectPath(ta,cur,success);
		if (success) return;
		ta.Set(cur.x,cur.y,0);
		cur.y-=1;
	}
}
void DrawSize(int width,int height)
{
	int left=textwidth(prompt2)+24;
	char w[]="Width:";
	char h[]=" Height:";
	char buffer[8];
	setfillstyle(SOLID_FILL,BGCOLOR);
	bar(left,472,left+textwidth(w)+textwidth(h)+textwidth(" ")*6,479);
	settextjustify(LEFT_TEXT,CENTER_TEXT);
	setcolor(GREEN);
	outtextxy(left,476,w);
	sprintf(buffer,"%3d",width);
	left+=textwidth(w);
	setcolor(BGCOLOR);
	outtextxy(left,476,"   ");
	setcolor(YELLOW);
	outtextxy(left,476,buffer);
	left+=textwidth(buffer);
	setcolor(GREEN);
	outtextxy(left,476,h);
	left+=textwidth(h);
	setcolor(BGCOLOR);
	outtextxy(left,476,"   ");
	sprintf(buffer,"%3d",height);
	setcolor(YELLOW);
	outtextxy(left,476,buffer);
}
void Draw()
{
	int style,size;
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	cleardevice();
	size = 1;
	style=TRIPLEX_FONT;
	settextstyle(style, HORIZ_DIR, size);
	outtextxy(320,9,caption);
	style=DEFAULT_FONT;
	settextstyle(style, HORIZ_DIR, size);
	settextjustify(LEFT_TEXT, CENTER_TEXT);
	outtextxy(24,467,prompt1);
	outtextxy(24,476,prompt2);
}
void DrawMaze(const Maze & m)
{
	int style,size,i,j,border;
	int mw,mh,left,top;
	setfillstyle(SOLID_FILL,BGCOLOR);
	bar(0,24,639,456);
	mw=m.GetWidth();
	mh=m.GetHeight();
	left=LEFT;
	top=TOP;
	setcolor(MAZECOLOR);
	line(left,top,left+mw*MAZE_CELL_SIZE,top);
	line(left,top,left,top+mh*MAZE_CELL_SIZE);
	for (j=0;j<mh;j++)
		for (i=0;i<mw;i++)
		{
			border=m.GetMaze(i,j);
			border=~border;
			if ( border & 0x1) /* bottom */
				line(left+(i+0)*MAZE_CELL_SIZE,
					top+(j+1)*MAZE_CELL_SIZE,
					left+(i+1)*MAZE_CELL_SIZE,
					top+(j+1)*MAZE_CELL_SIZE);
			if ( border & 0x2) /* right */
				line(left+(i+1)*MAZE_CELL_SIZE,
					top+(j+0)*MAZE_CELL_SIZE,
					left+(i+1)*MAZE_CELL_SIZE,
					top+(j+1)*MAZE_CELL_SIZE);
		}
}

void DrawPath(const Maze & m)
{
	int i,j;
	int mw,mh,left,top;
	mw=m.GetWidth();
	mh=m.GetHeight();
	left=LEFT;
	top=TOP;

	setcolor(PATHCOLOR);
	for (j=0;j<mh;j++) /* horz */
		for (i=0;i<mw-1;i++)
		{
			if (m.GetPath(i,j)&&m.GetPath(i+1,j)&& (m.GetMaze(i,j) & 0x2 ) )
				line(left+(i+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					top+(j+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					left+(i+1)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					top+(j+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
		}
	for (j=0;j<mh-1;j++)
		for (i=0;i<mw;i++)
		{
			if (m.GetPath(i,j)&&m.GetPath(i,j+1)&& (m.GetMaze(i,j) & 0x1 ) )
				line(left+(i+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					top+(j+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					left+(i+0)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
					top+(j+1)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
		}
}

int TraceMaze(TwoDimensionArray & tr,Point & cur,const Maze & m,direction dir)
{
	int left,top;
	left=(639-m.GetWidth()*MAZE_CELL_SIZE)/2;
	top=(479-m.GetHeight()*MAZE_CELL_SIZE)/2+2;;
	switch(dir)
	{
	case west:
		if (cur.x>0 && (m.GetMaze(cur.x-1,cur.y) & 0x2))
		{
			if (tr.Get(cur.x-1,cur.y)==0)
			{
				tr.Set(cur.x-1,cur.y,1);
				setcolor(TRACECOLOR);
			}
			else
			{
				tr.Set(cur.x,cur.y,0);
				setcolor(BADCOLOR);
			}
			line(left+((cur.x-1))*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
			cur.x-=1;
		}
		break;
	case north:
		if (cur.y>0 && (m.GetMaze(cur.x,cur.y-1) & 0x1))
		{
			if (tr.Get(cur.x,cur.y-1)==0)
			{
				tr.Set(cur.x,cur.y-1,1);
				setcolor(TRACECOLOR);
			}
			else
			{
				tr.Set(cur.x,cur.y,0);
				setcolor(BADCOLOR);
			}
			line(left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+(cur.y-1)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
			cur.y-=1;
		}
		break;
	case east:
		if (cur.x<m.GetWidth() && (m.GetMaze(cur.x,cur.y) & 0x2))
		{
			if (tr.Get(cur.x+1,cur.y)==0)
			{
				tr.Set(cur.x+1,cur.y,1);
				setcolor(TRACECOLOR);
			}
			else
			{
				tr.Set(cur.x,cur.y,0);
				setcolor(BADCOLOR);
			}
			line(left+(cur.x+1)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
			cur.x+=1;
		}
		break;
	case south:
		if (cur.y<m.GetHeight() && (m.GetMaze(cur.x,cur.y) & 0x1))
		{
			if (tr.Get(cur.x,cur.y+1)==0)
			{
				tr.Set(cur.x,cur.y+1,1);
				setcolor(TRACECOLOR);
			}
			else
			{
				tr.Set(cur.x,cur.y,0);
				setcolor(BADCOLOR);
			}
			line(left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+(cur.y+1)*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				left+cur.x*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2,
				top+cur.y*MAZE_CELL_SIZE+MAZE_CELL_SIZE/2);
			cur.y+=1;
		}
		break;
	}
	if (cur.x==m.GetEnd().x&&cur.y==m.GetEnd().y)
	{
		sound(700);
		delay(100);
		nosound();
		return 1;
	}
	return 0;
}


int main(void)
{
	int mazewidth=8,mazeheight=6,success=0;
	int oldw,oldh;
	randomize();
	TwoDimensionArray trace(mazewidth,mazeheight);
	Point cur(0,0);
	Maze m(mazewidth,mazeheight);
	int i,j,key;



	trace.Set(0,0,1);

	Draw();
	DrawMaze(m);
	DrawSize(m.GetWidth(),m.GetHeight());
	for(;;)
	{
		oldw=mazewidth;
		oldh=mazeheight;
		while (bioskey(1)==0);
		key=bioskey(0);
		switch(key)
		{
		case ESC:
			goto exit;
		case F1:
			DrawPath(m);
			break;
		case F2:
			m.SetSize(mazewidth,mazeheight);
		case F3:
			DrawMaze(m);
			trace.SetSize(mazewidth,mazeheight);
			trace.Set(0,0,1);
			cur.x=0;
			cur.y=0;
			success=0;
			break;
		case HOME:
			if (mazewidth>2) mazewidth--;
			break;
		case END:
			if (mazewidth<MAZE_MAX_WIDTH) mazewidth++;
			break;
		case PAGEUP:
			if (mazeheight>2) mazeheight--;
			break;
		case PGDOWN:
			if (mazeheight<MAZE_MAX_HEIGHT) mazeheight++;
			break;
		case LEFTKEY:
			if (!success)
				success=TraceMaze(trace,cur,m,west);
			break;
		case UPKEY:
			if (!success)
				success=TraceMaze(trace,cur,m,north);
			break;
		case RIGHTKEY:
			if (!success)
				success=TraceMaze(trace,cur,m,east);
			break;
		case DOWNKEY:
			if (!success)
				success=TraceMaze(trace,cur,m,south);
			break;
		}
		if (oldw!=mazewidth||oldh!=mazeheight)
			DrawSize(mazewidth,mazeheight);
	}
	exit:

	return 0;
}