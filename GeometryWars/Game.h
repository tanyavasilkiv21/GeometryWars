#pragma once

#include "imgui.h"
#include "imgui-SFML.h"
#include "EntityManager.h"
#include "SFML/Graphics.hpp"

struct PlayerConfig { int SR, CR, S, FR, FG, FB, OR, OG, OB, OT, V; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SP, SMIN, SMAX;  };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
	sf::RenderWindow m_window;
	int m_wWidth = 1280;
	int m_wHeight = 720;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	sf::Clock m_deltaClock;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	int m_spawnTime = 70;
	int m_framesForSpecialWeapon = 100;
	int m_framesPastForWeapon = 100;

	bool m_running = true;
	bool m_sMovement = true;
	bool m_sLifespawn = true;
	bool m_sEnemySpawner = true;
	bool m_sCollision = true;
	bool m_sRender = true;
	bool m_sUserInput = true;
	bool m_paused = false;
	std::shared_ptr<Entity> m_player;
public:
	Game(const std::string& config);
	void init(const std::string& path);
	void run();
	void setPaused(bool paused);
	void handlePauseInput();
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> e);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & target);
	void spawnSuperWeapon();
	void sMovement();
	void sCollision();
	void sEnemySpawner();
	void sGUI();
	void sLifespan();
	void sRender();
	void sUserInput();
};

