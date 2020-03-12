
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// In order to load the resources like cute_image.png, you have to set up
// your target scheme:
//
// - Select "Edit Scheme…" in the "Product" menu;
// - Check the box "use custom working directory";
// - Fill the text field with the folder path containing your resources;
//        (e.g. your project folder)
// - Click OK.
//

#include <iostream>
#include <cmath>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Window {
private:
    sf::RenderWindow window;
    sf::Vector2u windowSize;
    std::string title;
    bool closeWindow;
    bool fullScreen;
    void clear() {window.close();}
    void setup(const std::string &title, const sf::Vector2u &size);
    void create();
    
public:
    Window() {Window("Default", sf::Vector2u(600, 800));}
    Window(std::string title, sf::Vector2u windowSize) {setup(title, windowSize);}
    ~Window() {clear();}
    void clearDraw() {window.clear(sf::Color::Black);}
    void draw(sf::Drawable &drawable) {window.draw(drawable);}
    void display() {window.display();}
    void Update();
    bool getCloseWindow() {return closeWindow;}
    bool getFullScreen() {return fullScreen;}
    sf::Vector2u getWindowSize() {return windowSize;}
    sf::RenderWindow* getWindow() {return &window;}
    void toggleFullScreen();
};

void Window::setup(const std::string &title, const sf::Vector2u &size) {
    this->title = title;
    this->windowSize = size;
    closeWindow = false;
    fullScreen = false;
    create();
}

void Window::create() {
    auto style = (fullScreen ? sf::Style::Fullscreen : sf::Style::Default);
    window.create({windowSize.x, windowSize.y, 32}, title, style);
}

void Window::Update() {
    sf::Event e;
    while(window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) closeWindow = true;
    }
}

void Window::toggleFullScreen() {
    fullScreen = !fullScreen;
    clear();
    create();
}

enum class Direction{None, Left, Right};

class Breakout {
private:
    Direction dir;
//    sf::Vector2u position;
    int speed;
    int lives;
    int score;
    bool gameover;
    sf::RectangleShape board;
    
public:
    Breakout();
    ~Breakout(){};
    void render(sf::RenderWindow &window);
    void move(float elapsed);
    void tick(float elapsed);
    void setDirection(Direction dir){this->dir = dir;}
    Direction getDirection() {return dir;}
    void setScore(){score += 20;}
    void setLives(){lives--;}
    void lose() {gameover = true;}
    bool getGameStatus() {return gameover;}
    sf::RectangleShape getBoard() {return board;}
};

Breakout::Breakout() {
    board.setSize(sf::Vector2f(150.0f, 20.0f));
//    board.setOrigin(board.getSize().x / 2, board.getSize().y / 2);
    board.setPosition(600 - board.getSize().x / 2, 700);
    gameover = false;
    score = 0;
    lives = 3;
    speed = 15;
    setDirection(Direction::None);
}

void Breakout::render(sf::RenderWindow &window) {
    board.setFillColor(sf::Color::Red);
    window.draw(board);
}

void Breakout::move(float elapsed) {
    if (dir == Direction::Left && board.getPosition().x >= 0)
        board.setPosition(board.getPosition().x - (speed * elapsed), board.getPosition().y);
    else if (dir == Direction::Right &&
             (board.getPosition().x + board.getSize().x) <= 1200)
        board.setPosition(board.getPosition().x + (speed * elapsed), board.getPosition().y);
}

void Breakout::tick(float elapsed) {
    if (dir == Direction::None) return;
    move(elapsed);
}

using BlockSet = std::vector<sf::RectangleShape>;

class Board {
private:
    Breakout breakout;
    sf::CircleShape ball;
    std::vector<BlockSet> block;
    int blockNum;
    sf::Vector2i increment;
    void initialBlock();
    sf::Clock clock;
    float elapsed;
    
public:
    Board();
    ~Board(){}
    void Update();
    void handleInput();
    void ballMovement();
    void render(sf::RenderWindow &window);
    void collision();
    sf::Time getElapsed() {return clock.getElapsedTime();}
    void restartClock() {elapsed += clock.restart().asSeconds();}
    bool gameover() {return breakout.getGameStatus();}
};

void Board::initialBlock() {
    int marginY = 50, gapY = 10, gapX = 20;
    for (int y = 0; y < block.size(); y++) {
        int marginX = 130;
        for (int x = 0; x < block[y].size(); x++) {
            block[y][x].setSize(sf::Vector2f(100.0f, 20.0f));
            block[y][x].setFillColor(sf::Color::Blue);
            block[y][x].setPosition(marginX, marginY);
            marginX += gapX + block[y][0].getSize().x;
        }
        marginY += gapY + block[y][0].getSize().y;
    }
}

Board::Board() : block(6, BlockSet(8)) {
    initialBlock();
    ball.setRadius(12);
    ball.setOrigin(ball.getRadius(), ball.getRadius());
    ball.setPosition(600, 700 - 50);
    blockNum = block.size() * block[0].size();
    increment = sf::Vector2i(5, 5);
    clock.restart();
    elapsed = 0.0f;
}

void Board::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        breakout.setDirection(Direction::Left);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        breakout.setDirection(Direction::Right);
    else breakout.setDirection(Direction::None);
}

void Board::Update() {
    handleInput();
    ballMovement();
    breakout.tick(elapsed);
    float timestep = 1.0f / 10;
    if (elapsed >= timestep) {
        elapsed -= timestep;
    }
}

void Board::ballMovement() {
    int maxrangeY = block.back().front().getPosition().y;
    int blockSizeX = block.front().front().getSize().x;
    int blockSizeY = block.front().front().getSize().y;
    
//    border collision
    if (ball.getPosition().x - ball.getRadius() <= 0 ||
        ball.getPosition().y - ball.getRadius() <= 0 ||
        ball.getPosition().x + ball.getRadius() >= 1200) {
        if (ball.getPosition().x - ball.getRadius() <= 0 ||
            ball.getPosition().x + ball.getRadius() >= 1200) increment.x = -increment.x;
        if (ball.getPosition().y - ball.getRadius() <= 0) increment.y = -increment.y;
    }
//    ball enters the block range
    else if (ball.getPosition().y - ball.getRadius() <= maxrangeY + blockSizeY) collision();
//    check if ball hit the board
    else {
        float testX = ball.getPosition().x, testY = ball.getPosition().y;
        if (testX < breakout.getBoard().getPosition().x)
            testX = breakout.getBoard().getPosition().x;
        else if (testX > breakout.getBoard().getPosition().x +
                 breakout.getBoard().getSize().x)
            testX = breakout.getBoard().getPosition().x +
            breakout.getBoard().getSize().x;
        if (testY < breakout.getBoard().getPosition().y)
            testY = breakout.getBoard().getPosition().y;
        else if (testY > breakout.getBoard().getPosition().y +
                 breakout.getBoard().getSize().y)
            testY = breakout.getBoard().getPosition().y +
            breakout.getBoard().getSize().y;
        float distX = ball.getPosition().x - testX;
        float distY = ball.getPosition().y - testY;
        float distance = sqrt((distX * distX) + (distY * distY));
        if (distance <= ball.getRadius()) {
            if (testX != ball.getPosition().x &&
                testY != ball.getPosition().y) {
                increment.x = -increment.x;
                increment.y = -increment.y;
            }
            else if (testX == ball.getPosition().x) increment.y = -increment.y;
            else if (testY == ball.getPosition().y) increment.x = -increment.x;
        }
    }
    ball.setPosition(ball.getPosition().x + (increment.x * elapsed),
                     ball.getPosition().y + (increment.y * elapsed));
    if (ball.getPosition().y - ball.getRadius() >= 800) breakout.lose();
}


void Board::render(sf::RenderWindow &window) {
    for (int y = 0; y < block.size(); y++)
        for (int x = 0; x < block[y].size(); x++)
            window.draw(block[y][x]);
    window.draw(ball);
    breakout.render(window);
}

void Board::collision() {
    int blockSizeX = block.front().front().getSize().x;
    int blockSizeY = block.front().front().getSize().y;
    for (int y = 0; y < block.size(); y++) {
        for (int x = 0; x < block[y].size(); x++) {
            float testX = ball.getPosition().x, testY = ball.getPosition().y;
//            ball is at left to the block, testX = the posX of block
            if (testX < block[y][x].getPosition().x) testX =
                block[y][x].getPosition().x;
//            ball is at right to the block, testX = the posX + width of block
            else if (testX > block[y][x].getPosition().x + blockSizeX)
                testX = block[y][x].getPosition().x + blockSizeX;
//            ball is at top to the block, testY = the posY of block
            if (testY < block[y][x].getPosition().y) testY =
                block[y][x].getPosition().y;
//            ball is at bottom to the block, testY = the posY + height of block
            else if (testY > block[y][x].getPosition().y + blockSizeY)
                testY = block[y][x].getPosition().y + blockSizeY;
            
            float distX = ball.getPosition().x - testX;
            float distY = ball.getPosition().y - testY;
            float distance = sqrt((distX * distX) + (distY * distY));
            if (distance <= ball.getRadius()) {
//                collision happens!
                block[y].erase(block[y].begin() + x);
//                ball hit the corner of block
                if (testX != ball.getPosition().x && testY != ball.getPosition().y) {
                    increment.x = -increment.x;
                    increment.y = -increment.y;
                }
//                ball hit from the top or bottom to the block
                else if (testX == ball.getPosition().x) increment.y = -increment.y;
//                ball hit from the left or right to the block
                else if (testY == ball.getPosition().y) increment.x = -increment.x;
                return;
            }
        }
    }
}

int main(int argc, char const** argv) {
    Window m_window("Hello World", sf::Vector2u(1200, 800));
    Board world;
    while(!world.gameover() && !m_window.getCloseWindow()) {
        m_window.Update();
        m_window.clearDraw();
        world.Update();
        world.render(*m_window.getWindow());
        m_window.display();
        m_window.clearDraw();
        world.restartClock();
    }
    
    return EXIT_SUCCESS;
}
