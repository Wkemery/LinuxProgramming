/*************************************
 * This file is for a simple analyzer that converts a simplified version
 * of of latex and html into a file that can be parsed by latex and generates a
 * "beamer" presentation. Maybe not the best, but it does work. Made a couple of
 * poor choices but I can at least make it work. 
 * One of the main problems is including latex comments. I seem to have buggered
 * that up quite badly. Will see what more I can do to fix it. Probably really
 * should use Bison to do the rest of the conversion/decision making.
 *
 * Author: Kim Buckner
 * Date:
 * Last Modified: 10/02/11
 * 
 * Nove 21, 2013: Modified so that the beamer command "\againframe" ends the
 * previous frame/slide, prints yytext, and begins INITIAL 
 */
%option noyywrap
%{
char tbuf[512];
int level[15];
char localoptions[512];
int lcount;
int frame_start;
int current_state;
%}
WS	[ \t]
%x SECTION FRAME FSTART FBODY ITEMS NITEMS HEADER COMMENT INPUT OPTIONS

%%
<*>\\%	{fprintf(yyout,"%s",yytext);}
<*>%.*\n	{fprintf(yyout,"%s",yytext);}
<*>\<input\>	{current_state=YY_START; BEGIN(INPUT);}
<INPUT>[ \t]*   {/* eat whitespace*/}
<INPUT>[^ \t\n]+ { /*new filename*/
	yyin=fopen(yytext,"r");
	if(!yyin) {
	  perror("cannot open 'input' filename");
	  exit(1);
	}
	/*See the FLEX manual for discussion of multiple input buffers*/
	yypush_buffer_state(yy_create_buffer(yyin,YY_BUF_SIZE));
	BEGIN(current_state);
	}
\<addtooptions\> {BEGIN(OPTIONS);}
\<start\> {strcat(localoptions,"]{beamer}\n");
           fprintf(yyout,"%s",localoptions);}
\<title\> {fprintf(yyout,"\\title{");BEGIN(HEADER);}
\<sect\> {fprintf(yyout,"\\section{");BEGIN(SECTION);}
\<slide\> {BEGIN(FSTART);}
<FSTART>\[[^]]*\] {frame_start=1;fprintf(yyout,"\\begin{frame}%s\n",yytext);}
<FSTART>. 	{ if(! frame_start) {
		    fprintf(yyout,"\\begin{frame}\n");
		  }
		  fprintf(yyout,"\\frametitle{"); 
		  yyless(0);BEGIN(FRAME);
                }
<HEADER>[^<\n]+  {
		  fprintf(yyout,"%s}\n",yytext);
		  fprintf(yyout,"\\begin{document}\n");
		}
<HEADER>\n	{BEGIN(INITIAL);}
<HEADER>\<	{yyless(0);BEGIN(INITIAL);}
<SECTION>[^<\n]+ {fprintf(yyout,"%s",yytext);}
<SECTION>\n	{fprintf(yyout,"}\n");BEGIN(INITIAL);}
<SECTION>\< 	{yyless(0);BEGIN(INITIAL);}
<FRAME>[^\n]+   {fprintf(yyout,"%s",yytext);}
<FRAME>\n	{fprintf(yyout,"}\n");BEGIN(FBODY);}
<FBODY>\<ul\> 	{
		  fprintf(yyout,"\\begin{itemize}\n");
		  level[lcount++]=FBODY;
		  BEGIN(ITEMS);
		}
<FBODY>\<ol\> 	{
		  fprintf(yyout,"\\begin{enumerate}\n");
		  level[lcount++]=FBODY;
		  BEGIN(ITEMS);
		}
<FBODY>\<sect\>	{
		  fprintf(yyout,"\\end{frame}\n");
		  frame_start=0;
		  yyless(0);
		  BEGIN(INITIAL);
		}
<FBODY>\<slide\> {
		  fprintf(yyout,"\\end{frame}\n");
		  frame_start=0;
		  yyless(0);
		  BEGIN(INITIAL);
		}
<FBODY>\\againframe[^\n]*\n {
		  fprintf(yyout,"\\end{frame}\n");
		  frame_start=0;
		  fprintf(yyout,"%s\n",yytext);
		  BEGIN(INITIAL);
		}
<ITEMS>\<li\> {fprintf(yyout,"\\item ");}
<ITEMS>\<ul\> 	{
		  fprintf(yyout,"\\begin{itemize}\n");
		  level[lcount++]=ITEMS;
		  BEGIN(ITEMS);
		}
<ITEMS>\<ol\> 	{
		  fprintf(yyout,"\\begin{enumerate}\n");
		  level[lcount++]=ITEMS;
		  BEGIN(ITEMS);
		}
<ITEMS>\<\/ul\> {
		  fprintf(yyout,"\\end{itemize}\n");
		  BEGIN(level[--lcount]);
		}
<ITEMS>\<\/ol\> {
		  fprintf(yyout,"\\end{enumerate}\n");
		  BEGIN(level[--lcount]);
		}
<ITEMS>.	{fprintf(yyout,"%s",yytext);}
<OPTIONS>[^\n]* {strcat(localoptions,","); strcat(localoptions,yytext);}
<OPTIONS>\n	{BEGIN(INITIAL);}
	/*
<COMMENT>[^\n]*	{fprintf(yyout,"%s",yytext);}
<COMMENT>\n	{BEGIN(level[lcount-1]);}
	*/
<<EOF>>		{
	/*Again see FLEX manual for multiple input buffers*/
	yypop_buffer_state();
	if(!YY_CURRENT_BUFFER) {
	  fprintf(yyout,"\\end{frame}\n");return 0;
	  yyterminate();
	  }
	}
%%

int main(int argc,char**argv)
{
  FILE *in,*out;
  
  yyout=stdout;
  yyin =stdin;

  if(argc >= 2) {
    in = fopen(argv[1],"r");
    if(in == NULL) {
      perror(argv[1]);
      return 1;
    }
    /*printf("opened %s\n",argv[1]);*/
    yyin=in;
  }
  if(argc >=3 ) {
    out=fopen(argv[2],"w");
    if(out== NULL) {
      perror(argv[2]);
      return 1;
    }
    yyout=out;
  }
  /*
  else {
    printf("yyout = stdout \n");
  }
  */
  lcount=0;
  sprintf(localoptions,"\\documentclass[17pt");
  //fprintf(yyout,"\\documentclass[17pt]{beamer}\n");
  yylex();
  fprintf(yyout,"\\end{document}\n");
  return 0;
}

