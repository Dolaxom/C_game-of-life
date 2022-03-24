#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MAX_ROWS 25
#define MAX_COLUMNS 80

void Initialize(int ***matrix, int rows, int columns);
void FillInitialState(int InitialStateArray[MAX_ROWS][MAX_COLUMNS], int **matrix);
int DrawField(int **matrix, int rows, int columns);
int DeadOrAlive(int count, int isAlive);
int CountNeighbors(int **matrix, int n, int m, int i, int j);
void LifeStep(int **NowState, int **NewState, int n, int m);
void ChangeStep(int ***NowState, int ***NewState);
int ReadInitialStateFile(int argc, char *argv[], int InitialStateArray[MAX_ROWS][MAX_COLUMNS]);
char clickListener(int *timeDelay);


int main(int argc, char *argv[]) {
    int rows = MAX_ROWS;
    int columns = MAX_COLUMNS;
    int timeDelay = 100000;
    int IsFileReadOk = 1;
    int InitialStateArray[MAX_ROWS][MAX_COLUMNS] = {0};
    int **NowState, **NewState;
    char pressedKey = ' ';

    IsFileReadOk = ReadInitialStateFile(argc, argv, InitialStateArray);

    if (IsFileReadOk) {
        Initialize(&NowState, rows, columns);
        Initialize(&NewState, rows, columns);
        FillInitialState(InitialStateArray, NowState);
    }

    while (IsFileReadOk && pressedKey != 'q') {
        printf("\e[H\e[2J\e[3J");
        DrawField(NowState, rows, columns);
        LifeStep(NowState, NewState, rows, columns);
        ChangeStep(&NowState, &NewState);
        pressedKey = clickListener(&timeDelay);
        usleep(timeDelay);
    }

    if (IsFileReadOk) {
        free(NowState);
        free(NewState);
    }
    return 0;
}

void Initialize(int ***matrix, int rows, int columns) {
    int * ptr;

    *matrix = (int **)malloc(rows * columns * sizeof(int) + rows * sizeof(int*));
    ptr = (int*)(*matrix + rows);

    for (int i = 0; i < rows; i++) {
        (*matrix)[i] = ptr + columns * i;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            (*matrix)[i][j] = 0;
        }
    }
}

int DrawField(int **matrix, int rows, int columns) {
    char Border, Point;
    int BorderLeftX, BorderRightX, BorderUpY, BorderDownY;

    Border = '#';
    BorderLeftX = -1;
    BorderRightX = columns;
    BorderUpY = -1;
    BorderDownY = rows;

    for (int i = BorderUpY; i <= BorderDownY; i++) {
        for (int j = BorderLeftX; j <= BorderRightX; j++) {
            if (i == BorderUpY || i == BorderDownY
                || j == BorderLeftX || j == BorderRightX ) {
                    printf("%c", Border);
            } else {
                Point = (matrix[i][j] == 1) ? '*' : ' ';
                printf("%c", Point);
            }
        }
        printf("\n");
    }
    return 1;
}

int CountNeighbors(int **matrix, int n, int m, int i, int j) {
    int count;
    int NeighborI, NeighborJ;
    count = 0;
    for (int k = i - 1; k <= i + 1; k++) {
        for (int l = j - 1;  l <= j + 1; l++) {
            NeighborI = (n + k) % n;
            NeighborJ = (m + l) % m;
            if (matrix[NeighborI][NeighborJ] == 1
                && !(NeighborI == i && NeighborJ == j)
                ) {
                 count += 1;
            }
        }
    }
    return count;
}

int DeadOrAlive(int count, int isAlive) {
    if (isAlive == 1) {
        isAlive = (count == 2 || count == 3) ? 1 : 0;
    } else {
        isAlive = (count == 3) ? 1 : 0;
    }
    return isAlive;
}

void LifeStep(int **NowState, int **NewState, int n, int m) {
    int count, isAlive;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            isAlive = NowState[i][j];
            count = CountNeighbors(NowState, n, m, i, j);
            NewState[i][j] = DeadOrAlive(count, isAlive);
        }
    }
}

void FillInitialState(int InitialStateArray[MAX_ROWS][MAX_COLUMNS], int **matrix) {
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLUMNS; j++) {
            matrix[i][j] = InitialStateArray[i][j];
        }
    }
}

void ChangeStep(int ***NowState, int ***NewState) {
    int **ptr;
    ptr = *NowState;
    *NowState = *NewState;
    *NewState = ptr;
}

int ReadInitialStateFile(int argc, char *argv[], int InitialStateArray[MAX_ROWS][MAX_COLUMNS]) {
    int i, j;
    int FscanfState = 2;
    int FunctionState = 1;

    if (argc != 2) {
        printf("You should add file path to fill initial state.\n");
        FunctionState = 0;
    }

    if (FunctionState) {
        FILE *file;
        file = fopen(argv[1], "r");
        if (file) {
            while (!feof(file) && FscanfState == 2) {
                FscanfState = fscanf(file, "%d %d", &i, &j);
                if (i < 0 || i >= MAX_ROWS || j < 0 || j >= MAX_COLUMNS) {
                    printf("Error: indexes are outside permissible values.\n");
                    FunctionState = 0;
                } else {
                    InitialStateArray[i][j] = 1;
                }
            }
            fclose(file);
        } else {
            printf("Error: could not open file `%s`.\n", argv[1]);
            FunctionState = 0;
        }
    }

    if (FscanfState != 2 && FunctionState) {
        printf("File format is incorrect.\n");
        FunctionState = 0;
    }
    return FunctionState;
}

char clickListener(int *timeDelay) {
    int is_key_pressed;

    int timeShift = 20000;
    int minTimeDelay = 400000;
    char pressedKey = ' ';

    system("/bin/stty raw");
    ioctl(1, FIONREAD, &is_key_pressed);

    if (is_key_pressed) {
        pressedKey = getchar();
    }

    if (*timeDelay < minTimeDelay && (pressedKey == '-' || pressedKey == '_')) {
        *timeDelay += timeShift;
    } else if (*timeDelay > timeShift && (pressedKey == '=' || pressedKey == '+')) {
        *timeDelay -= timeShift;
    }
    system("/bin/stty cooked");
    return pressedKey;
}
