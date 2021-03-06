// SudokuX
// Release 1.5
// Arguments: File containing Sudoku Puzzle Strings
// Output: Ascii graphic of last puzzle solved and percentSolved solved on exit
// Author: Jonathan Balls

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>

#include "sudokux.h"
#include "gamesolver.h"

main(int argc, char *argv[]) {
	int			i, j;

	// Number of puzzles solved and failed.
	int			solved =		0;

	// Total number of problems to be solved.
	int			problemCount =	0;

	// Percentage of puzzles solved
	float 		percentSolved;

	// Variable to hold filename of sudoku puzzles
	char		filename[MAX_FILENAME_LENGTH];

	// Struct that contains all the preferences.
	struct settings preferences;

	// Check whether an argument has been passed and whether it contains an
	// accessible filename.
	if (!argCheck(argc, argv, &preferences, filename))
		return;

	// Gets all _valid_ sudoku puzzles from the file and puts them into a
	// large array.
	int **problemList = fileTo2DArray(filename, &problemCount);

	struct windows window;
	
	// Start ncurses and create windows etc.
	if (preferences.cursesMode) {
		prepareterminal(filename, &window);
	}

	for (i=0; i<problemCount; i++) {
			if (preferences.cursesMode) {
				displayPuzzle(window.unsolved, problemList[i]);
			}
		switch (solveSudoku(problemList[i])){
			case SOLVED:
				solved++;
				break;
			case FAILED:
				break;
		}
		if (preferences.cursesMode) {
			displayPuzzle(window.solved, problemList[i]);
			updateStatistics(i, solved, problemCount, window.statistics);
		}
	}

	// End ncurses screen
	if (preferences.cursesMode) {
		endwin();
	}

	// Print stats and percentages
	percentSolved = solved;
	percentSolved = 100 * percentSolved / problemCount;
	printf("%d (%.2f%%) solved out of %d\n",
		solved, percentSolved, problemCount);

	return 0;
}


// returns a pointer to a 2D array of all the sudoku puzzles in 
int **fileTo2DArray(char filename[MAX_FILENAME_LENGTH], int *problemCount) {
	int 		i;

	// Pointer to the array of pointers to arrays of ints.
	int**		problemList;

	int 		linelength;
	char 		*line =		NULL;
	size_t		len =		0;

	// Create a file pointer and open the file of sudoku puzzles.
	FILE 		*fp;
	fp = fopen(filename, "r");

	// Find the maximum number of puzzles stored in a file by dividing filesize
	// by the size of a puzzle (plus one for \n and EOF characters)
	fseek(fp, 0, SEEK_END);
	int maxpuzzles = ftell(fp) / (SUDOKU_CELLS + 1);
	fseek(fp, 0, SEEK_SET);

	// Allocate a 2D array using information based on the max size of the array.
	problemList = (int**) malloc(maxpuzzles * sizeof(int*));
	for (i=0; i<maxpuzzles; i++) {
		problemList[i] = malloc(SUDOKU_CELLS * sizeof(int));
	}

	while ((linelength = getline(&line, &len, fp)) != -1) {

		// Length of standard sudoku puzzle and the newline/EOF character.
		if (linelength != SUDOKU_CELLS + 1) {
			continue;
		}

		// Translate ascii numbers to their dec equivalents in the problemList
		for (i=0; i<SUDOKU_CELLS; i++) {
			if (line[i] == EMPTY_CELL) {
				problemList[*problemCount][i] = 0;
			}
			else {
				problemList[*problemCount][i] = line[i] - '0';
			}
		}
		*problemCount = *problemCount + 1;
	}

	// free up allocated space for storing individual lines and close the file
	free(line);
	fclose(fp);

	return problemList;
}

// Parses argument flags and filename errors
argCheck(int argc, char *argv[], struct settings *preferences, char filename[MAX_FILENAME_LENGTH]) {
	int i;

	if (!(argc-1)) {
		printf("Err: Argument required\n");
		return 0;
	}

	if (strcmp(argv[1], "-h") == 0) {
		printVersionNumber();
		printf("\n");
		printf("USAGE: sudokux [OPTION]... [FILE]\n");
		printf("\n");
		printf("OPTIONS:\n");
		printf("    -s    Silent mode. Curses mode is not displayed.\n");
		printf("    -v    Print the version number\n");
		printf("    -h    Display this help message\n");
		return 0;
	}

	if (strcmp(argv[1], "-v") == 0){
		printVersionNumber();
		return 0;
	}

	// There needs to be a better way to set defaults.
	preferences->cursesMode = DEFAULT_CURSES_MODE;

	// Cycle through arguments which aren't filename or command name
	for (i=1; i<argc-1; i++) {
		if (strcmp(argv[i], "-s") == 0) {
			preferences->cursesMode = false;
		} else {
			printf("Invalid command entered: %s\n", argv[i]);
			return 0;
		}
	}

	// argv[argc - 1] is where filename should be.
	strcpy(filename, argv[argc-1]);

	// Check that the filename is actually accessible
	if(access(filename, R_OK ) == -1) {
		printf("Fatal Error: File does not exists\n");
		return 0;
	}

	return 1;
}

void printVersionNumber() {
	printf("SudokuX version: %.2f\n", SUDOKUX_VERSION_NUMBER);
}

void prepareterminal(char filename[MAX_FILENAME_LENGTH], struct windows* window) {
	// Get the centre of the terminal screen
	initscr();

	curs_set(0);
	noecho();

	// Print filename at the top of the window
	mvprintw(0, (COLS / 2) - (strlen(filename)/2), filename);

	refresh();

	// Create a window for unsolved puzzle. It is positioned according to
	// constants defined in the header file. They are all annotated so
	// feel free to change what you see fit.
	WINDOW *unsolvedwin;
	unsolvedwin = newwin(ASCII_HEIGHT, ASCII_WIDTH, ASCII_TOP_MARGIN,
		(COLS / 2) - (ASCII_WIDTH + (ASCII_CENTRE_MARGIN / 2)));
	wrefresh(unsolvedwin);

	// See above comment. Notice that this window is placed to the write
	// instead of the left.
	WINDOW *solvedwin;
	solvedwin = newwin(ASCII_HEIGHT, ASCII_WIDTH, ASCII_TOP_MARGIN,
		(COLS / 2) + (ASCII_CENTRE_MARGIN / 2));
	wrefresh(solvedwin);

	// Window for the statistics below the puzzles.
	WINDOW *statisticswin;
	statisticswin = newwin(NUMBER_OF_STATISTICS, STATISTICS_WIDTH,
		ASCII_TOP_MARGIN + ASCII_HEIGHT, (COLS / 2) - (STATISTICS_WIDTH / 2));

	window->unsolved = unsolvedwin;
	window->solved	 = solvedwin;
	window->statistics = statisticswin;

	return;
}


// Displays a given puzzle in the window. The window is assumed to be of
// correct size. This function needs to be rewritten to avoid assumptions and
// display puzzles of other sizes than 9x9.
// >muh magic numbers
void displayPuzzle(WINDOW *puzzleWin, int sudokupuzzle[SUDOKU_CELLS]) {
	int i;
	struct coordinates coord = {0, 0};
	mvwprintw(puzzleWin, ++coord.y, 0,"+ - - - + - - - + - - - +");
	mvwprintw(puzzleWin, ++coord.y, 0, "| ");
	for (i=0;i<SUDOKU_CELLS;i++) {
		if (sudokupuzzle[i] == 0) {
			wprintw(puzzleWin, ". ");
		} else {
			wprintw(puzzleWin, "%d ", sudokupuzzle[i]);
		}
		if ((i+1)%(SUDOKU_COLUMNS / SUDOKU_BOX_COLUMNS) == 0) {
			if ((i+1)%SUDOKU_ROWS == 0) {
				if ((i+1)%(SUDOKU_BOX_ROWS*SUDOKU_ROWS) == 0) {
					if ((i+1)%SUDOKU_CELLS == 0) {
						wprintw(puzzleWin, "|");
						mvwprintw(puzzleWin, ++coord.y, 0,"+ - - - + - - - + - - - +");
						mvwprintw(puzzleWin, ++coord.y, 0, "| ");
						continue;
					}
					wprintw(puzzleWin, "|");
					mvwprintw(puzzleWin, ++coord.y, 0, "+ - - - + - - - + - - - +");
					mvprintw(++coord.y, 0, "| ");
					continue;
				}
				wprintw(puzzleWin, "|");
				mvwprintw(puzzleWin, ++coord.y, 0, "| ");
				continue; 
			}
			wprintw(puzzleWin, "| ");
			continue;
		}
	}
	wrefresh(puzzleWin);

	return;
}

void updateStatistics(int attempts, int solved, int problemCount, WINDOW *statistics) {
	int failed = (attempts+1) - solved;
	int remaining = problemCount - attempts;
	int currenty = 0;
	int centre = STATISTICS_WIDTH / 2;
	werase(statistics);

	mvwprintw(statistics, ++currenty, centre-16, "Sudoku Number:    %d", attempts+1);
	mvwprintw(statistics, ++currenty, centre-9, "Solved:    %d", solved);
	mvwprintw(statistics, ++currenty, centre-9, "Failed:    %d", failed);
	mvwprintw(statistics, ++currenty, centre-12, "Remaining:    %d", (remaining)-1);
	wrefresh(statistics);

	return;
}