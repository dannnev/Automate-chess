#include <SFML/Graphics.hpp>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace sf;

int size = 100; // 1/8-th of a board 800x800 px
int fish = 0, end = 0, draw = 0; // fish indicates the start of engine, end indicates that engine has finished
int halfmove = 0, fullmove = 1; // two last quantities of FEN string
int translation [4]; // translation of chess move (examp. string "c3d4" -> {2,5,3,6,})
int repeated = 0;

bool side = true; // side = white is to move

char metamorph = '0'; // when pawn promotion happen, metamorph captures the figure it promotes to

Sprite f[12]; //figures for dragging to the board
Sprite wh[23]; //white placed figures
Sprite bl[23]; //black placed figures
Sprite g[12]; //green squares - highlight allowed pieces to drag
Sprite y[2]; //yellow squares - highlight current move on the board

std::string position=""; // moves history
std::string fen = ""; //FEN is used to load the current position to stockfish
std::string themove = ""; // stores the best engine move
std::string enpassant = ""; // shows enpassant
std::string blackdraw [10] = {"","","","","","","","","",""};
std::string whitedraw [10] = {"","","","","","","","","",""};
std::string lastblackfen = "";
std::string lastwhitefen = "";

// list of letters describing pieces according to FEN notation
char tabular [12] = { 'k', 'q', 'b', 'n', 'r', 'p', 'K', 'Q', 'B', 'N', 'R', 'P'};

// symbol board that stores current position and is also used to create FEN
char board[8][8] = {
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
     '0', '0', '0', '0', '0', '0', '0', '0',
};

// trace of figures that are currently playing. Used to visualise the current position
int ghostboard[8][8] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

// list of permissions to drag pieces to the board at the start of the game. Helps to enforce Rules
bool permission [12] = {
    false, false, false, false, false, true,
    false, false, false, false, false, true,
};

// legit variable is used to confirm that the pieces are dragged to an untaken square of the board
bool legit = false; 

// Hence the following function decides whether the position (newPos) is legit
bool onBoardCheck (Vector2f newPos) 
{
    if ((newPos.x < 8*size) && (newPos.y < 8*size) && (board[int (newPos.y / size)][int(newPos.x / size)] == '0'))
        legit=true;
    else
        legit=false;
    return legit;
}

// FEN generator
std::string generateFEN(char board[8][8], bool side, int halfmove, int fullmove)
{
    int k;
    fen = "";
    std::string W = " w - ", B = " b - ";
    for (int i=0;i<8;i++)
        {
            k = 0;
            for (int j=0;j<8;j++)
            {
                if (board[i][j] == '0') k++;
                else
                    {
                        if (k>0) fen += std::to_string(k);
                        fen += board[i][j];
                        k=0;
                    }
                if (j == 7 && k>0) fen += std::to_string(k);
            }
            if(i<7) fen += '/';
        }
    
    if (side) {lastblackfen = fen; fen += W;}
    else {lastwhitefen = fen; fen += B;}
    if (enpassant != "") {fen += enpassant; enpassant="";}
    else fen += "- ";
    fen += std::to_string(halfmove);
    fen += ' ';
    fen += std::to_string(fullmove);
    return fen;
}

// draw by repetition check
void drawcheck (std::string a)
{
    if (side) for (int i=0; i<10; i++) if (blackdraw[i] == lastblackfen) repeated++;
    if (!side) for (int i=0; i<10;i++) if (whitedraw[i] == lastwhitefen) repeated++;
    if (repeated > 2) draw=1;
    else
    {
        if (side)
        {
            for (int i=9; i>0; i--) blackdraw[i]=blackdraw[i-1];
            blackdraw[0]=lastblackfen;
        }
        else
        {
            for (int i=9; i>0; i--) whitedraw[i]=whitedraw[i-1];
            whitedraw[0]=lastwhitefen;
        }
        repeated=0;
    }    
}


// engine finds the best move
void stockfish()
{
    fen = generateFEN(board,side,halfmove,fullmove);
    std::ofstream fentxt ("./fen.txt", std::ios::trunc);
    fentxt << fen;
    fentxt.close();
    if (fullmove<15) system("./hardthink.sh");
    if (14<fullmove && fullmove<30) system("./middlethink.sh");
    if (29<fullmove) system("./easythink.sh");
    system("./sponge.sh");
    std::fstream filemoves ("./script.txt");
    filemoves >> themove;
    filemoves.close();
    if (themove[0] == '(') {fish=0; end=1;}
    if (halfmove == 50) draw=1;
    drawcheck(fen);
    if (draw) {fish=0; end=1;}
}

// translates chess move to window coordinates
void translate(std::string themove)
{
    for (int i=0;i<3;i+=2)
        switch (themove[i]){
            case ('a'):
                translation[i]=0;
                break;
            case ('b'):
                translation[i]=1;
                break;
            case ('c'):
                translation[i]=2;
                break;
            case ('d'):
                translation[i]=3;
                break;
            case ('e'):
                translation[i]=4;
                break;
            case ('f'):
                translation[i]=5;
                break;
            case ('g'):
                translation[i]=6;
                break;
            case ('h'):
                translation[i]=7;
                break;
        }
    for (int i=1;i<4;i+=2)
        switch (themove[i]){
            case ('1'):
                translation[i]=7;
                break;
            case ('2'):
                translation[i]=6;
                break;
            case ('3'):
                translation[i]=5;
                break;
            case ('4'):
                translation[i]=4;
                break;
            case ('5'):
                translation[i]=3;
                break;
            case ('6'):
                translation[i]=2;
                break;
            case ('7'):
                translation[i]=1;
                break;
            case ('8'):
                translation[i]=0;
                break;
        }
    metamorph = themove[4];
}

int main()
{
   RenderWindow window(VideoMode(1600, 800), "Automate chess"); // creates the window of 1600x800 px size

   // Textures are the "sources" for Sprite class. Only 3 textures are set - one for background, one for pieces, one for green squares
   // By switching the files that a texture t2 is loaded from, the different background appears.
   Texture t1,t2,t3,t4;
   t1.loadFromFile("../images/figures.png"); // figures texture
   t2.loadFromFile("../images/starter.png"); // starter page
   t3.loadFromFile("../images/greensquare.png"); // green squares represent allowed to drag figures
   t4.loadFromFile("../images/yellowsquare.png"); // yellow squares for the previous move

   Sprite mainframe(t2); // mainframe is of class Sprite, hence it is now drawable

   // Set font for text
   Font actionman;
   actionman.loadFromFile("../fonts/actionman.ttf");

   // points to spend on pieces
   int amoB=35, amoW=35;
   
   // Set text for points left
   Text blscore("Points " + std::to_string(amoB),actionman,80) ,whscore("Points " + std::to_string(amoW),actionman,80);
   blscore.setPosition(10*size,2*size);
   blscore.setFillColor(Color::Black);
   whscore.setPosition(10*size,5*size);
   
   // set the text for the winner
   Text blwin("     Black Won\nPress P to restart",actionman,50), whwin("     White Won\nPress P to restart",actionman,50);
   blwin.setPosition(10*size,3.5*size);
   blwin.setFillColor(Color::Black);
   whwin.setPosition(10*size,3.5*size);

   // text if draw is the result
   Text resultisdraw("        Draw\nPress P to restart",actionman,50);
   resultisdraw.setPosition(10*size,3.5*size);

   // set two rows of drawable pieces
   // recall that lines 8-10 set the f[i] as Sprite, but it is not yet drawn until we set the "source" - t1 texture
   for(int i=0;i<12;i++)
    {
        if (i<6) //black
            {f[i].setTextureRect( IntRect(size*i, size,size,size) ); // figures.png is 12 figures simultaneously, hence with cut through it a square size x size px
            f[i].setPosition(size*(9+i),size);
            g[i].setPosition(size*(9+i),size);}
        else //white
            {f[i].setTextureRect( IntRect(size*(i-6), 0,size,size) );
            f[i].setPosition(size*(3+i),size*6);
            g[i].setPosition(size*(3+i),size*6);}
        g[i].setTexture(t3);
    }

    // yellow squares
    y[0].setTexture(t4);
    y[0].setPosition(Vector2f(11*size,3.5*size));
    y[1].setTexture(t4);
    y[1].setPosition(Vector2f(12*size,3.5*size));

   // fix prevents entering game if you are currently reading rules
   // r helps loading the rules at any moment of the game
   // enter is a one time indicator of pressing enter button
   // isMove helps creating an image of actually dragging a piece across the window
   bool isMove=false, r=true, enter=true, fix=true, turn=true, backspace=false;

   // differences of positions
   float dx=0, dy=0;

   // Vector2 is a class of tuples (x,y). Vector2f stands for float, Vector2i stands for int
   Vector2f oldPos,newPos;

   // n determines the piece from f[n], n is declared with value > 12 to avoid bugs
   // w and b are counters for how many white/black pieces were placed.
   // row, file are the coordinates of a square on a chessboard
   int n=13, p=0, l=0, w=0, b=0, row, file, lastrow, lastfile;
   int blpawns=0, whpawns=0;
   int fallblack = 0, fallwhite = 0;

   // horizontalls for fallen figures
   int hor1=0, hor2=0;

   // onces are used in && conditions to make it happen only once
   int once1 = 1, once2 = 1, once3 = 1, once4 = 1;

   while (window.isOpen())
   {
        // capture the coordinates of mouse pointer in the window
        Vector2i pos = Mouse::getPosition(window);
        
        // e is an event of player interacting with the game
        Event e;
        while (window.pollEvent(e))
        {
            // close game 
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed)
                if (e.key.code == Keyboard::Escape)
                    window.close();

            // press the R button for rules
            if (e.type == Event::KeyPressed)
                if(e.key.code == Keyboard::R)
                    if (r)
                        {t2.loadFromFile("../images/rules.png"); //change the texture of the mainframe to the prepared rules.png
                         r=false;
                         fix = false; // prevents entering the game from the rules window
                        }
                    else
                        {
                            if (!enter)
                                t2.loadFromFile("../images/board.png"); // if enter was already used we move in between game and rules
                            else
                                t2.loadFromFile("../images/starter.png"); // if enter was not yet used we move in between game and starter window
                            r=true;
                            fix = true; // allows entering the game
                        }

            // Enter button to play
            if (e.type == Event::KeyPressed)
                if(e.key.code == Keyboard::Enter)
                    if (enter && fix)
                        {
                            t2.loadFromFile("../images/board.png"); // load board
                            for (int i=0; i<12; i++) f[i].setTexture(t1); // finally set f[i] as texture t1
                            enter = false;
                        }
            
            // comeback button backspace
            if (e.type == Event::KeyPressed)
                if(e.key.code == Keyboard::BackSpace)
                    if(!enter && backspace){
                        file = lastfile;
                        row = lastrow;
                        switch (n){
                            case 0:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once4) turn = !turn;
                                    once3 = 1;
                                }
                                break;
                            case 1:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    amoB = amoB + 7;
                                    if (once4) turn = !turn;
                                }
                                break;
                            case 2:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    amoB = amoB + 3;
                                    if (once4) turn = !turn;
                                }
                                break;
                            case 3:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    amoB = amoB + 3;
                                    if (once4) turn = !turn;
                                }
                                break;
                            case 4:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    amoB = amoB + 4;
                                    if (once4) turn = !turn;
                                }
                                break;
                            case 5:
                                {
                                    bl[b].setPosition(Vector2f(1600,0));
                                    b--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    amoB = amoB + 1;
                                    if (once4) turn = !turn;
                                    blpawns--;
                                }
                                break;
                            case 6:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    once4=1;
                                }
                                break;
                            case 7:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    amoW = amoW + 7;
                                }
                                break;
                            case 8:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    amoW = amoW + 3;
                                }
                                break;
                            case 9:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    amoW = amoW + 3;
                                }
                                break;
                            case 10:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    amoW = amoW + 4;
                                }
                                break;
                            case 11:
                                {
                                    wh[w].setPosition(Vector2f(1600,0));
                                    w--;
                                    ghostboard[row][file]=0;
                                    board[row][file]='0';
                                    if (once3) turn = !turn;
                                    amoW = amoW + 1;
                                    whpawns--;
                                }
                                break;
                        }
                        blscore.setString("Points " + std::to_string(amoB));
                        whscore.setString("Points " + std::to_string(amoW));
                        backspace=false;
                    }

            // Play again
            if (end)
                if (e.type == Event::KeyPressed)
                    if (e.key.code == Keyboard::P)
                        {
                            end=0;
                            once1=1;
                            once2=1;
                            once3=1;
                            once4=1;
                            amoB=35;
                            amoW=35;
                            n=13;p=0;
                            b=0;
                            w=0;
                            fallwhite=0;
                            fallblack=0;
                            hor1=0;
                            hor2=0;
                            blpawns=0;
                            whpawns=0;
                            turn=true;
                            side = !side;
                            permission[5]=true;
                            permission[11]=true;
                            for(int s=1;s<23;s++) {wh[s].setPosition( Vector2f(1600,0) ); bl[s].setPosition( Vector2f(1600,0) );}
                            for (int i=0;i<8;i++)
                                for (int j=0;j<8;j++)
                                    {
                                        board[i][j]='0';
                                        ghostboard[i][j]='0';
                                    }
                            blscore.setString("Points " + std::to_string(amoB));
                            whscore.setString("Points " + std::to_string(amoW));
                            oldPos=Vector2f(1600,0);
                        }
                
            // drag the pieces
            if (e.type == Event::MouseButtonPressed)
                if (e.mouseButton.button == Mouse::Left)
                    if (turn)
                        {
                            for (int i=6;i<12;i++)
                                if (permission[i])
                                    if (f[i].getGlobalBounds().contains(pos.x,pos.y))
                                    {
                                        isMove=true; n=i;
                                        dx=pos.x - f[i].getPosition().x;
                                        dy=pos.y - f[i].getPosition().y;
                                        oldPos  =  f[i].getPosition();
                                    }
                        }
                    else
                        {
                            for (int i=0;i<6;i++)
                                if (permission[i])
                                    if (f[i].getGlobalBounds().contains(pos.x,pos.y))
                                    {
                                        isMove=true; n=i;
                                        dx=pos.x - f[i].getPosition().x;
                                        dy=pos.y - f[i].getPosition().y;
                                        oldPos  =  f[i].getPosition();
                                    }
                        }

            // drop the piece
            if (e.type == Event::MouseButtonReleased)
                if (e.mouseButton.button == Mouse::Left)
                 {
                  // stops mouse moving 
                  isMove=false;

                  Vector2f p = f[n].getPosition() + Vector2f(size/2,size/2);
                  newPos = Vector2f( size*int(p.x/size), size*int(p.y/size) );

                  // capture file and row
                  file = int (newPos.x / size);
                  row = int (newPos.y / size);

                  // duplicate file and row
                  lastfile = file;
                  lastrow = row;

                  // complete the rule of placing 6 pawns firstly
                  if (b>5 && once1) {permission[1]=true; permission[2]=true; permission[3]=true; permission[4]=true; once1=0;}
                  if (w>5 && once2) {permission[7]=true; permission[8]=true; permission[9]=true; permission[10]=true; once2=0;}

                  // only 12 pawns
                  if (blpawns>11) permission[5] = false;
                  if (whpawns>11) permission[11] = false;

                  // onBoardCheck
                  legit = onBoardCheck(newPos);

                  //maintain the rules of placing figures
                 if (legit)
                  {
                    switch (n){
                        case 0:
                            if (row == 1 || row == 0)
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='k';
                                    if (once4) turn = !turn;
                                    once3 = 0;
                                }
                            break;
                        case 1:
                            if ((row == 1 || row == 0) && (amoB >= 7))
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='q';
                                    amoB = amoB - 7;
                                    if (once4) turn = !turn;
                                }
                            break;
                        case 2:
                            if ((row == 1 || row == 0) && (amoB >= 3))
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='b';
                                    amoB = amoB - 3;
                                    if (once4) turn = !turn;
                                }
                            break;
                        case 3:
                            if ((row == 1 || row == 0) && (amoB >= 3))
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='n';
                                    amoB = amoB - 3;
                                    if (once4) turn = !turn;
                                }
                            break;
                        case 4:
                            if ((row == 1 || row == 0) && (amoB >= 4))
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='r';
                                    amoB = amoB - 4;
                                    if (once4) turn = !turn;
                                }
                            break;
                        case 5:
                            if ((row == 1 || row == 2) && (amoB >= 1))
                                {
                                    b++;
                                    bl[b].setTexture(t1);
                                    bl[b].setTextureRect(IntRect(size*n, size,size,size));
                                    bl[b].setPosition(newPos);
                                    ghostboard[row][file]=b;
                                    board[row][file]='p';
                                    amoB = amoB - 1;
                                    if (once4) turn = !turn;
                                    blpawns++;
                                }
                            break;
                        case 6:
                            if (row == 6 || row == 7)
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='K';
                                    once4 = 0;
                                    if (once3) turn = !turn;
                                }
                            break;
                        case 7:
                            if ((row == 6 || row == 7) && (amoW >= 7))
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='Q';
                                    amoW = amoW - 7;
                                    if (once3) turn = !turn;
                                }
                            break;
                        case 8:
                            if ((row == 6 || row == 7) && (amoW >= 3))
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='B';
                                    amoW = amoW - 3;
                                    if (once3) turn = !turn;
                                }
                            break;
                        case 9:
                            if ((row == 6 || row == 7) && (amoW >= 3))
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='N';
                                    amoW = amoW - 3;
                                    if (once3) turn = !turn;
                                }
                            break;
                        case 10:
                            if ((row == 6 || row == 7 ) && (amoW >= 4))
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='R';
                                    amoW = amoW - 4;
                                    if (once3) turn = !turn;
                                }
                            break;
                        case 11:
                            if ((row == 6 || row == 5) && (amoW >= 0))
                                {
                                    w++;
                                    wh[w].setTexture(t1);
                                    wh[w].setTextureRect(IntRect(size*(n-6), 0,size,size));
                                    wh[w].setPosition(newPos);
                                    ghostboard[row][file]=w;
                                    board[row][file]='P';
                                    amoW = amoW - 1;
                                    if (once3) turn = !turn;
                                    whpawns++;
                                }
                            break;
                    }
                    backspace=true;
                  }
                 
                 // update the points left
                 blscore.setString("Points " + std::to_string(amoB));
                 whscore.setString("Points " + std::to_string(amoW));

                 // when all points are spent we place kings
                 if (amoB == 0)
                    {
                        permission[0]=true; permission[1]=false; permission[2]=false;
                        permission[3]=false; permission[4]=false; permission[5]=false;
                    }
                 if (amoW == 0)
                    {
                        permission[6]=true; permission[7]=false; permission[8]=false;
                        permission[9]=false; permission[10]=false; permission[11]=false;
                    }

                 // return drawable piece
                 f[n].setPosition(oldPos);
                 
                 // bring to life the stockfish module
                 if (!once3 && !once4) {fish = 1; permission[0]=false; permission[6]=false;}

                 // block black pieces if there is not enough points left
                 if (amoB - 7 < 0) permission[1]=false;
                 if (amoB - 4 < 0) permission[4]=false;
                 if (amoB - 3 < 0) {permission[3]=false; permission[2]=false;}

                 // block white pieces if there is not enough points left
                 if (amoW - 7 < 0) permission[7]=false;
                 if (amoW - 4 < 0) permission[10]=false;
                 if (amoW - 3 < 0) {permission[9]=false; permission[8]=false;}
                }   
        }

        // changes the position of currently dragged piece to create a view of actually dragging it across the board
        if (isMove) f[n].setPosition(pos.x-dx,pos.y-dy);

        // drawing
        window.clear();
        window.draw(mainframe);

        // draw green squares as hints and points left
        if (!enter)
            {
                if (turn)
                    for (int i=6;i<12;i++)
                        if (permission[i]) window.draw(g[i]);
                if (!turn)
                    for (int i=0;i<6;i++)
                        if (permission[i]) window.draw(g[i]);
                if (once3) window.draw(blscore);
                if (once4) window.draw(whscore);
            }

        // draw the winner
        if (end)
            {
                if (!draw)
                    {
                        if (side) window.draw(blwin);
                        else window.draw(whwin);
                    }
                else window.draw(resultisdraw);
            }

        // placed figures
        for(int s=1;s<23;s++) {window.draw(wh[s]); window.draw(bl[s]);}

        if (once3) for(int i=0;i<6;i++) window.draw(f[i]); // remove drawable black pieces after placing the black king
        if (once4) for(int i=6;i<12;i++) window.draw(f[i]); // remove drawable white pieces after placing the white king
        if (!r) window.draw(mainframe); //to show rules on top during game
        window.display();

        // stockfish plays the game
        while (fish)
            {
                stockfish();
                if (end) goto label;
                translate(themove);
                oldPos=Vector2f(size*translation[0],size*translation[1]);
                newPos=Vector2f(size*translation[2],size*translation[3]);
                p=ghostboard[translation[3]][translation[2]];
                l=ghostboard[translation[1]][translation[0]];
                if (board[translation[1]][translation[0]] == 'p' && translation[1] == 1 && translation[3] == 3) enpassant = themove[0] + "6";
                if (board[translation[1]][translation[0]] == 'P' && translation[1] == 6 && translation[3] == 4) enpassant = themove[0] + "3";
                if (side)
                    {
                        if (translation[3] == 0 && board[translation[1]][translation[0]] == 'P')
                            {
                                wh[l].setPosition(newPos);
                                switch(metamorph)
                                    {
                                        case ('q'):
                                            wh[l].setTextureRect(IntRect(size, 0,size,size));
                                            board[translation[1]][translation[0]]= 'Q';
                                            break;
                                        case ('b'):
                                            wh[l].setTextureRect(IntRect(2*size, 0,size,size));
                                            board[translation[1]][translation[0]]= 'B';
                                            break;
                                        case ('n'):
                                            wh[l].setTextureRect(IntRect(3*size, 0,size,size));
                                            board[translation[1]][translation[0]]= 'N';
                                            break;
                                        case ('r'):
                                            wh[l].setTextureRect(IntRect(4*size, 0,size,size));
                                            board[translation[1]][translation[0]]= 'R';
                                            break;
                                    }
                            }
                        else
                            {
                                wh[l].setPosition(newPos);
                            }
                        if (p>0) {bl[p].setPosition(Vector2f(size*(9+0.5*fallblack),(6+hor1)*size)); fallblack++; if (fallblack>11) {hor1++; fallblack=0;}}
                        window.display();
                    }
                else
                    {
                        if (translation[3] == 7 && board[translation[1]][translation[0]] == 'p')
                            {
                                bl[l].setPosition(newPos);
                                switch(metamorph)
                                    {
                                        case ('q'):
                                            bl[l].setTextureRect(IntRect(size, size,size,size));
                                            board[translation[1]][translation[0]]= 'q';
                                            break;
                                        case ('b'):
                                            bl[l].setTextureRect(IntRect(2*size, size,size,size));
                                            board[translation[1]][translation[0]]= 'b';
                                            break;
                                        case ('n'):
                                            bl[l].setTextureRect(IntRect(3*size, size,size,size));
                                            board[translation[1]][translation[0]]= 'n';
                                            break;
                                        case ('r'):
                                            bl[l].setTextureRect(IntRect(4*size, size,size,size));
                                            board[translation[1]][translation[0]]= 'r';
                                            break;
                                    }
                            }
                        else
                            {
                                bl[l].setPosition(newPos);
                            }
                        if (p>0) {wh[p].setPosition(Vector2f(size*(9+0.5*fallwhite),(1+hor2)*size)); fallwhite++; if (fallwhite>11) {hor2--; fallwhite=0;}}
                        fullmove++;
                        window.display();
                    }
                 
                if (p>0 || board[translation[1]][translation[0]] == 'p' || board[translation[1]][translation[0]] == 'P') halfmove=0;
                else halfmove++;
                y[0].setPosition(newPos);
                y[1].setPosition(oldPos);
                board[translation[3]][translation[2]]=board[translation[1]][translation[0]];
                board[translation[1]][translation[0]]='0';
                ghostboard[translation[3]][translation[2]]=l;
                ghostboard[translation[1]][translation[0]]=0;
                side = !side;
                label:
                window.clear();
                window.draw(mainframe);
                window.draw(y[0]); 
                window.draw(y[1]);
                for(int s=1;s<23;s++) {window.draw(wh[s]); window.draw(bl[s]);}
                window.display();
            }
        
    }

    return 0;
}