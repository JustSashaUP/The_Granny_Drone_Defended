#include "Game.h"
#include "Map.h"
#include "Text.h"
#include "Sound.h"
#include "Vector2D.h"
#include "Collision.h"
#include "Components.h"
#include "TextureManager.h"

//std::vector<Entity> destEnemies;

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;

Manager manager;
Manager menuManager;
Manager jarManager;
Manager enemiManager;
Manager leaderManager;

/// <summary>
/// Colors
/// </summary>
SDL_Color color_yellow = { 255, 255, 0, 255 };
SDL_Color color_white = { 255, 255, 255, 255 };
SDL_Color color_green = { 0, 255, 0, 255 };
SDL_Color color_purple = { 255, 0, 255, 255 };

/// <summary>
/// Map creating
/// </summary>
Map* garden_mp;

/// <summary>
/// Text creating
/// </summary>
Text* score_tx;
Text* scoreCounter_tx;
Text* liveCounter_tx;
Text* totalScore_tx;
Text* gameover_tx;
Text* retryButton_tx;
Text* exitButton_tx;

/// <summary>
/// Player and object creating
/// </summary>
auto& granny_player(manager.addEntity());
auto& jar(jarManager.addEntity());
auto& enemi(enemiManager.addEntity());
auto& heart(manager.addEntity());
auto& wallTop(manager.addEntity());
auto& wallBottom(manager.addEntity());
auto& leader1(leaderManager.addEntity());
auto& leader2(leaderManager.addEntity());
auto& leader3(leaderManager.addEntity());
auto& leader4(leaderManager.addEntity());

Mix_Music* gameMusic = Sound::PlayMusic("assets/sounds/game.mp3");
Mix_Music* menuMusic = Sound::PlayMusic("assets/sounds/menu.mp3");

Game::Game()
{}
Game::~Game()
{}
 
void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
	int flags = 0;
	if (fullscreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		TTF_Init();
		std::cout << "Game Subsystem is initialised!..." << std::endl;

		window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (window)
		{
			std::cout << "Window is created!" << std::endl;
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			std::cout << "Renderer created!" << std::endl;
		}
		Mix_PlayMusic(menuMusic, -1);
		isRunning = showMenu();
		Mix_FreeMusic(menuMusic);
	}

	Mix_PlayMusic(gameMusic, -1);

	//Map initialization
	garden_mp = new Map("assets/garden_mp.png");

	score_tx = new Text("SCORE", "assets/Sample_font.ttf", color_yellow, 32, 128, 32, 1050, 0);
	scoreCounter_tx = new Text(std::to_string(scoreCounter), "assets/Sample_font.ttf", color_white, 32, 32, 32, 1200, 0);
	liveCounter_tx = new Text("x" + std::to_string(liveCounter), "assets/Sample_font.ttf", color_white, 32, 64, 32, 920, 0);
	totalScore_tx = scoreCounter_tx;
	gameover_tx = new Text("Game over", "assets/Sample_font.ttf", color_white, 128, 1024, 256, 300, 200);
	retryButton_tx = new Text("Retry", "assets/Sample_font.ttf", color_white, 22, 64, 32, 1050, 550);
	exitButton_tx = new Text("Exit", "assets/Sample_font.ttf", color_white, 32, 64, 32, 450, 550);

	//Player initialization (adding Components)
	granny_player.addComponent<TransformComponent>(-1, 400);
	granny_player.addComponent<SpriteComponent>("assets/granny_player.png", true);
	granny_player.addComponent<KeyboardController>();
	granny_player.addComponent<ColliderComponent>("granny_player");

	//Leaders initialization (adding Components)
	leader1.addComponent<TransformComponent>(-20, 0, 168, 178, 1);
	leader1.addComponent<SpriteComponent>("assets/leader.png", true, 2);
	leader1.addComponent<ColliderComponent>("leader");
	leader2.addComponent<TransformComponent>(-20, 175, 168, 178, 1);
	leader2.addComponent<SpriteComponent>("assets/leader.png", true, 2);
	leader2.addComponent<ColliderComponent>("leader");
	leader3.addComponent<TransformComponent>(-20, 350, 168, 178, 1);
	leader3.addComponent<SpriteComponent>("assets/leader.png", true, 2);
	leader3.addComponent<ColliderComponent>("leader");
	leader4.addComponent<TransformComponent>(-20, 525, 168, 178, 1);
	leader4.addComponent<SpriteComponent>("assets/leader.png", true, 2);
	leader4.addComponent<ColliderComponent>("leader");

	//Jar initialization (adding Components)
	jar.addComponent<TransformComponent>(-500, 500, 64, 64, 1);
	jar.addComponent<SpriteComponent>("assets/glass_jar.png");
	jar.addComponent<ColliderComponent>("jar");

	//Enemi initialization (adding Components)
	enemi.addComponent<TransformComponent>(-500.0f, -500.0f, 64, 64, 1);
	enemi.addComponent<SpriteComponent>("assets/enemi.png");
	enemi.addComponent<ColliderComponent>("enemi");

	heart.addComponent<TransformComponent>(850, 0, 64, 64, 1);
	heart.addComponent<SpriteComponent>("assets/heart.png");
	heart.addComponent<ColliderComponent>("heart");

	//Walls initialization (adding Components)
	wallTop.addComponent<TransformComponent>(0, 0, 1600, 20, 1);
	wallTop.addComponent<SpriteComponent>();
	wallTop.addComponent<ColliderComponent>("wallTop");

	wallBottom.addComponent<TransformComponent>(0, 750, 1600, 20, 1);
	wallBottom.addComponent<SpriteComponent>();
	wallBottom.addComponent<ColliderComponent>("wallBottom");
}

void Game::render()
{
	if (isMenu == false)
	{
		SDL_RenderClear(renderer);
		garden_mp->DrawMap();
		score_tx->DrawText();
		scoreCounter_tx->DrawText();
		liveCounter_tx->DrawText();
		leaderManager.draw();
		manager.draw();
		jarManager.draw();
		enemiManager.draw();
		SDL_RenderPresent(renderer);
	}
}

void Game::update()
{
	counter++;
	std::cout << "Frame: " << counter << std::endl;
	if (isMenu == false)
	{
		leaderManager.refresh();
		leaderManager.update();
		manager.refresh();
		manager.update();
		jarManager.refresh();
		jarManager.update();
		enemiManager.refresh();
		enemiManager.update();
		collisionCheck();
	}
	else
	{
		isOver();
	}
}

void Game::isOver()
{
	uint64_t currentTime = 0;
	uint64_t spendTime = 0;
	const int FPS = 60;
	const int frameDelay = 1000 / FPS;
	int oneTime = 0;
	granny_player.getComponent<TransformComponent>().velocity.y = 0;
	while (isMenu)
	{
	SDL_RenderClear(renderer);
	garden_mp->DrawMap();
	gameover_tx->DrawText();
	totalScore_tx->DrawText();
	retryButton_tx->DrawText();
	exitButton_tx->DrawText();
	leaderManager.refresh();
	leaderManager.update();
	manager.refresh();
	manager.update();
	currentTime = SDL_GetTicks();
	counter++;
	while (SDL_PollEvent(&event))
	{
		int xb = event.button.x;
		int yb = event.button.y;
		int xm = event.motion.x;
		int ym = event.motion.y;
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			if (xb >= 1050 &&
				xb <= 1120 &&
				yb >= 550 &&
				yb <= 620)
			{
				// jar and enemi settings
				spendJarTime = 0;
				spendEnemiTime = 0;
				enemiCounter = 0;
				enemispeed = 0.5f;
				jarspeed = 5;
				oneTime = 10;
				// score settings
				scoreCounter = 0;
				scoreEnemiSpeedLimit = 200;
				scoreJarSpeedLimit = 2000;
				scoreLimitSize = 1000;

				liveCounter = 1;

				isMenu = false;
				isHarm = false;

				score_tx = new Text("SCORE", "assets/Sample_font.ttf", color_yellow, 32, 128, 32, 1050, 0);
				scoreCounter_tx = new Text(std::to_string(scoreCounter), "assets/Sample_font.ttf", color_white, 32, 32, 32, 1200, 0);
				liveCounter_tx = new Text("x" + std::to_string(liveCounter), "assets/Sample_font.ttf", color_white, 32, 64, 32, 920, 0);
				break;
			}
			if (xb >= 450 &&
				xb <= 520 &&
				yb >= 550 &&
				yb <= 620)
			{
				isRunning = false;
				isMenu = false;
				break;
			}
		case SDL_MOUSEMOTION:
			if (xm >= 1050 &&
				xm <= 1120 &&
				ym >= 550 &&
				ym <= 620)
			{
				retryButton_tx = new Text("Retry", "assets/Sample_font.ttf", color_purple, 22, 64, 32, 1050, 550);
				break;
			}
			if (xm >= 450 &&
				xm <= 520 &&
				ym >= 550 &&
				ym <= 620)
			{
				exitButton_tx = new Text("Exit", "assets/Sample_font.ttf", color_purple, 32, 64, 32, 450, 550);
				break;
			}
			else
			{
				retryButton_tx = new Text("Retry", "assets/Sample_font.ttf", color_white, 22, 64, 32, 1050, 550);
				exitButton_tx = new Text("Exit", "assets/Sample_font.ttf", color_white, 32, 64, 32, 450, 550);
				break;
			}
			break;
		case SDL_QUIT:
			isMenu = false;
			isRunning = false;
			break;
		}
	}
	SDL_RenderPresent(renderer);
	spendTime = SDL_GetTicks() - currentTime;
	if (frameDelay > spendTime)
	{
		SDL_Delay(frameDelay - spendTime);
	}
	std::cout << "Frame: " << counter << std::endl;
	}
}



bool Game::showMenu()
{
	auto& menu0(menuManager.addEntity());
	auto& menu1(menuManager.addEntity());
	auto& buttons0(menuManager.addEntity());
	auto& buttons1(menuManager.addEntity());
	uint64_t currentTime = 0;
	uint64_t spendTime = 0;
	const int FPS = 60;
	const int frameDelay = 1000 / FPS;
	int oneTime = 0;


	garden_mp = new Map("assets/garden_mp.png");

	menu0.addComponent<TransformComponent>(700, 300, 256, 128, 1);
	menu1.addComponent<TransformComponent>(700, 400, 256, 128, 1);
	buttons0.addComponent<TransformComponent>(1100, 270, 64, 192, 1);
	buttons1.addComponent<TransformComponent>(1200, 350, 128, 128, 1);
	menu0.addComponent<SpriteComponent>("assets/menu0.png");
	menu1.addComponent<SpriteComponent>("assets/menu1.png");
	buttons0.addComponent<SpriteComponent>("assets/buttons0.png");
	buttons1.addComponent<SpriteComponent>("assets/buttons1.png");

	while (isMenu)
	{
		SDL_RenderClear(renderer);
		garden_mp->DrawMap();
		menuManager.draw();
		menuManager.refresh();
		menuManager.update();
		currentTime = SDL_GetTicks();
		counter++;
		while (SDL_PollEvent(&event))
		{
			int xb = event.button.x;
			int yb = event.button.y;
			int xm = event.motion.x;
			int ym = event.motion.y;
			switch (event.type)
			{
			case SDL_QUIT:
				isMenu = false;
				return false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (xb >= 700 &&
					xb <= 900 &&
					yb >= 300 &&
					yb <= 400)
				{
					isMenu = false;
					std::cout << "Status:" << "Game is Running!" << std::endl;
					return true;
					break;
				}

				if (xb >= 700 &&
					xb <= 900 &&
					yb >= 401 &&
					yb <= 500)
				{
					isMenu = false;
					return false;
					break;
				}
				break;
			case SDL_MOUSEMOTION:
				if (xm >= 700 &&
					xm <= 900 &&
					ym >= 300 &&
					ym <= 400)
				{
					if (oneTime == 0)
					{
					Mix_Chunk* motion = Sound::LoadSound("assets/sounds/motion.wav");
					Mix_PlayChannel(-1, motion, 0);
					oneTime = 1;
					}
					menu0.addComponent<SpriteComponent>("assets/menu0_m.png");
					break;
				}

				else if (xm >= 700 &&
					xm <= 900 &&
					ym >= 410 &&
					ym <= 500)
				{
					if (oneTime == 0)
					{
						Mix_Chunk* motion = Sound::LoadSound("assets/sounds/motion.wav");
						Mix_PlayChannel(-1, motion, 0);
						oneTime = 1;
					}
					menu1.addComponent<SpriteComponent>("assets/menu1_m.png");
					break;
				}

				else
				{
					oneTime = 0;
					menu0.addComponent<SpriteComponent>("assets/menu0.png");
					menu1.addComponent<SpriteComponent>("assets/menu1.png");
					break;
				}
				break;
			}
		}
		SDL_RenderPresent(renderer);
		spendTime = SDL_GetTicks() - currentTime;
		if (frameDelay > spendTime)
		{
			SDL_Delay(frameDelay - spendTime);
		}
		std::cout << "Frame: " << counter << std::endl;
	}
}

void Game::collisionCheck()
{
	if (Collision::AABB(granny_player.getComponent<ColliderComponent>().collider,
		wallTop.getComponent<ColliderComponent>().collider) ||
		Collision::AABB(granny_player.getComponent<ColliderComponent>().collider,
			wallBottom.getComponent<ColliderComponent>().collider))
	{
		if (granny_player.getComponent<TransformComponent>().position.y >= 20)
		{
			granny_player.getComponent<TransformComponent>().position.y -= 9;
		}
		if (granny_player.getComponent<TransformComponent>().position.y < 20)
		{
			granny_player.getComponent<TransformComponent>().position.y += 9;
		}
	}

	for (auto& entityJarPtr : jarManager.getEntities())
	{
		for (auto& entityEnemiPtr : enemiManager.getEntities())
		{
			auto& entityJar = *entityJarPtr;
			auto& entityEnemi = *entityEnemiPtr;
			if (entityEnemi.getComponent<TransformComponent>().position.y >= 800 &&
				entityEnemi.getComponent<SpriteComponent>().currentAnimation == "flying")
			{
				if (liveCounter != 0)
				{
					entityEnemi.destroy();
					Mix_Chunk* harm = Sound::LoadSound("assets/sounds/harm.wav");
					Mix_PlayChannel(-1, harm, 0);
					liveCounter--;
					liveCounter_tx = new Text("x" + std::to_string(liveCounter), "assets/Sample_font.ttf", color_white, 32, 64, 32, 920, 0);
					totalScore_tx = new Text("Total Score: " + std::to_string(scoreCounter), "assets/Sample_font.ttf", color_yellow, 32, 512, 128, 520, 400);
					for (auto& entityJarPtr : jarManager.getEntities())
					{
						auto& everyJar = *entityJarPtr;
						everyJar.destroy();
					}
					for (auto& entityEnemiPtr : enemiManager.getEntities())
					{
						auto& everyEnemi = *entityEnemiPtr;
						everyEnemi.destroy();
					}
					isMenu = true;
				}
			}
			if (Collision::AABB(entityJar.getComponent<ColliderComponent>().collider,
				entityEnemi.getComponent<ColliderComponent>().collider))
			{
				Mix_Chunk* destroyedSound = Sound::LoadSound("assets/sounds/destroyed.wav");
				Mix_PlayChannel(-1, destroyedSound, 0);
				entityEnemi.getComponent<SpriteComponent>().Play("destroyed");
				entityEnemi.getComponent<TransformComponent>().velocity.Add(Vector2D(1, 0));
				entityJar.destroy();
				scoreCounter += 10;
				if (scoreCounter < 100)
				{
					scoreCounter_tx = new Text(std::to_string(scoreCounter), "assets/Sample_font.ttf", color_white, 32, 64, 32, 1200, 0);
				}
				else {
					if (scoreCounter >= scoreLimitSize)
					{
						scoreW += 1;
						scoreH += 1;
						scoreLimitSize *= 10;
					}
					if (scoreCounter > 999999)
					{
						scoreCounter = 0;
					}
					scoreCounter_tx = new Text(std::to_string(scoreCounter), "assets/Sample_font.ttf", color_white, 32, scoreW, scoreH, 1200, 0);
				}
			}
			else if (entityEnemi.getComponent<SpriteComponent>().currentAnimation == "destroyed")
			{
				auto& spriteComponent = entityEnemi.getComponent<SpriteComponent>();
				auto& transformComponent = entityEnemi.getComponent<TransformComponent>();

				int totalAnimationTime = spriteComponent.frames * spriteComponent.speed;
				totalcurrentTime++;

				if (totalcurrentTime >= totalAnimationTime)
				{
					entityEnemi.destroy();
					totalcurrentTime = 0;
				}
			}
		}
	}
}

// jar update function
// Every 2000 score jar speed increase on 1
void Game::jarUpdate()
{
	if (Game::event.key.keysym.sym == SDLK_SPACE)
	{
		if (currentTime - spendJarTime <= 400)
		{
			currentTime = SDL_GetTicks();
		}
		else {
			if (oneTime > 10)
			{
			Mix_Chunk* jar = Sound::LoadSound("assets/sounds/jar.wav");
			Mix_PlayChannel(-1, jar, 0);
			oneTime = 0;
			}
			oneTime++;
			if (scoreCounter >= scoreJarSpeedLimit)
			{
				scoreJarSpeedLimit = scoreCounter * 2;
				jarspeed += 1;
			}
			auto& newJar(jarManager.addEntity());
			newJar.addComponent<TransformComponent>(granny_player.getComponent<TransformComponent>().position.x,
				granny_player.getComponent<TransformComponent>().position.y, 64, 64, 1);
			newJar.addComponent<SpriteComponent>("assets/glass_jar.png", true, 0);
			newJar.addComponent<ColliderComponent>("newJar");
			newJar.getComponent<TransformComponent>().velocity.Add(Vector2D(jarspeed, 0));
			spendJarTime = currentTime;
		}
	}
}

// Enemi update function
// Enemi spawn at random place. Every 1000 score enemi speed increase on 1
void Game::enemiUpdate()
{
	if (currentTime - spendEnemiTime <= 2000)
	{
		currentTime = SDL_GetTicks();
	}
	else {
		if (scoreCounter >= scoreEnemiSpeedLimit)
		{
			scoreEnemiSpeedLimit = scoreCounter * 2;
			enemispeed += 0.2f;
		}
	auto& newEnemi(enemiManager.addEntity());
	newEnemi.addComponent<TransformComponent>(800, -100, 64, 64, 1);
	newEnemi.addComponent<SpriteComponent>("assets/enemi.png", true, 1);
	newEnemi.addComponent<ColliderComponent>("newEnemi");
	enemiCounter++;
	switch (enemiCounter)
	{
	case 2:
		newEnemi.getComponent<TransformComponent>().position.x += rand() % 700;
		newEnemi.getComponent<TransformComponent>().velocity.Add(Vector2D(0, enemispeed + 0.5f));
		break;
	case 4:
		newEnemi.getComponent<TransformComponent>().position.x += 200 + rand() % 101;
		newEnemi.getComponent<TransformComponent>().velocity.Add(Vector2D(0, enemispeed));
		enemiCounter = 0;
		break;
	default:
		newEnemi.getComponent<TransformComponent>().position.x += 100 + rand() % 101;
		newEnemi.getComponent<TransformComponent>().velocity.Add(Vector2D(0, enemispeed));
		break;
	}
	spendEnemiTime = currentTime;
	}
}

 void Game::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	std::cout << "Status:" << "Game Cleaned!" << std::endl;
}

void Game::handleEvents() 
{
	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_QUIT:
			isRunning = false;
			break;

		default:
			break;
	}
}