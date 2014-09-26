// SudokuX
// Release 1.5
// Arguments: File containing Sudoku Puzzle Strings
// Output: Ascii graphic of last puzzle solved and percentage solved on exit
// Author: Jonathan Balls

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

#include "gamesolver.c"

main(int argc, char *argv[]) {
	int i, j;
	int solved = 0, failed = 0;;
	int noOfProblems;
	float percentage;
	char filename[256];
	int currentproblem[81];

	if (!errCheck(argc, argv, filename))
		return;
	strcpy(filename, argv[1]);
	noOfProblems = getProblemCount(filename, argc);
	if (!noOfProblems) //ends program if there are no puzzles
		return;
	char problemList[noOfProblems][81]; //creates a big list for all the puzzles

	fileToStack(filename, problemList, noOfProblems); //takes all the puzzles and puts them into the big list.
	initscr(); //initialises screen for ncurses
	for (i=0;i<noOfProblems;i++) { //solves each sudoku
		printUnsolved(problemList, i, filename);
		setCurrentProblem(problemList, currentproblem, i); //takes a new problem from the list

		switch (solveSudoku(currentproblem)){ //tries to solve the puzzle and reacts accordingly
			case 1:
				solved++;
				break;
			case -1:
				failed++;
				break;
		}
		setSolvedProblem(problemList, currentproblem, i);
		printSolved(problemList, i, solved, failed, noOfProblems);
	}
	endwin();
	percentage = solved;
	percentage = 100 * percentage / noOfProblems;
	printf("%d(%.2f%%) solved out of %d\n", solved, percentage, noOfProblems);
	return 0;
}

setCurrentProblem(char problemList[][81],int currentproblem[81], int i) {
	int j;
	for (j=0;j<81;j++) {
		if (problemList[i][j] == '.') {
			currentproblem[j] = 0;
		}
		else {
			currentproblem[j] = problemList[i][j] - 48;
		}
	}
}

setSolvedProblem(char problemList[][81], int currentproblem[81], int i) {
	int j;
	for (j=0;j<81;j++) {
		if (currentproblem[j]==0) {
			problemList[i][j]='.';
		}
		else {
			problemList[i][j]= currentproblem[j] + 48;
		}
	}
}

errCheck(int argc, char *argv[]) { //checks for errors in the arguments
	if (!(argc-1)) {
		printf("Err: Argument required\n");
		return 0;
	}
	char filename[256];
	strcpy(filename, argv[1]);
	if(access( filename, R_OK ) == -1) {
		printf("Err: File does not exists\n");
		return 0;
	}
	return 1;
}

getProblemCount(char filename[256], int argc) { //accepts filename and returns linecount
	FILE *pf;
	char ch;
	int lines = 0;
	pf = fopen(filename, "r");
	ch = fgetc(pf);
	while (ch != EOF) {
		if (ch == '\n')
			lines++;
		ch = fgetc(pf);
	}
	fclose(pf);
	return lines;
}

fileToStack(char filename[256],char problemList[][81],int noOfProblems) {
	int i;
	FILE *pf;
	pf = fopen(filename, "r");

  for (i=0;i<noOfProblems;i++) {
  	fgets(problemList[i], 255, pf);
  }
  fclose(pf);
}

printUnsolved(char problemList[][81],int problem, char filename[256]) {
	clear();
	int i, row, col, startx;
	getmaxyx(stdscr,row,col);
	startx = col/2 - 27;
	int currenty = 2;

	mvprintw(0, (col/2) - (strlen(filename)/2), "%s", filename);//prints filename center top
	mvprintw(++currenty, startx,"+ - - - + - - - + - - - +");
	mvprintw(++currenty, startx, "| ");

  for (i=0;i<81;i++) {
    printw("%c ", problemList[problem][i]);
    if ((i+1)%3 == 0) {
      if ((i+1)%9 == 0) {
        if ((i+1)%27 == 0) {
          if ((i+1)%81 == 0) {
            printw("|");
            mvprintw(++currenty, startx, "+ - - - + - - - + - - - +");
            move(++currenty, startx);
            continue; }
          printw("|");
          mvprintw(++currenty, startx, "+ - - - + - - - + - - - +");
          mvprintw(++currenty, startx, "| ");
          continue; }
        printw("|");
        mvprintw(++currenty, startx, "| ");
        continue; }
      printw("| ");
      continue; }
  }
}

printSolved(char problemList[][81],int problem, int solved, int failed, int noOfProblems) {
	int i, row, col, startx;
	getmaxyx(stdscr,row,col);
	startx = col/2 + 2;
	int currenty = 2;

	mvprintw(++currenty, startx,"+ - - - + - - - + - - - +");
	mvprintw(++currenty, startx, "| ");

  for (i=0;i<81;i++) {
    printw("%c ", problemList[problem][i]);
    if ((i+1)%3 == 0) {
      if ((i+1)%9 == 0) {
        if ((i+1)%27 == 0) {
          if ((i+1)%81 == 0) {
            printw("|");
            mvprintw(++currenty, startx, "+ - - - + - - - + - - - +");
            move(++currenty, startx);
            continue; }
          printw("|");
          mvprintw(++currenty, startx, "+ - - - + - - - + - - - +");
          mvprintw(++currenty, startx, "| ");
          continue; }
        printw("|");
        mvprintw(++currenty, startx, "| ");
        continue; }
      printw("| ");
      continue; }
  }

  int centre = col/2;
  mvprintw(++currenty, centre-16, "Sudoku Number:    %d", problem+1);
  mvprintw(++currenty, centre-9, "Solved:    %d", solved);
  mvprintw(++currenty, centre-9, "Failed:    %d", failed);
  mvprintw(++currenty, centre-11, "Unsolved:    %d", (problem+1)-solved);
  mvprintw(++currenty, centre-12, "Remaining:    %d", (noOfProblems-problem)-1);
  refresh();
}