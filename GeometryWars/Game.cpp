#include "Game.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <random>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	std::ifstream fileIn(path);
	if (fileIn.is_open())
	{
		std::string line;
		while (std::getline(fileIn, line))
		{
			std::istringstream iss(line);
			std::string shapeType;
			if ((iss >> shapeType))
			{
				if (shapeType == "Window")
				{
					int framerate;
					iss >> m_wWidth >> m_wHeight >> framerate;
					m_window.create(sf::VideoMode(m_wWidth, m_wHeight), "Geometry Wars");
					m_window.setFramerateLimit(framerate);
				}
				if (shapeType == "Font")
				{
					std::string fontPath;
					int color[3];
					int size;
					iss >> fontPath >> size >> color[0] >> color[1] >> color[2];
					if (!m_font.loadFromFile(fontPath))
					{
						std::cerr << "couldn't load font\n";
						exit(-1);
					}
					m_text.setFont(m_font);
					m_text.setCharacterSize(size);
					m_text.setPosition(sf::Vector2f(50, 50));
					m_text.setFillColor(sf::Color(color[0], color[1], color[2]));
				}
				if (shapeType == "Player")
				{
					iss >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB;
					iss >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
				}
				if (shapeType == "Enemy")
				{
					iss >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB;
					iss >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SP;
					m_spawnTime = m_enemyConfig.SP;
				}
				if (shapeType == "Bullet")
				{
					iss >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB;
					iss >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
				}
			}
		}
		ImGui::SFML::Init(m_window);
		spawnPlayer();
	}
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		ImGui::SFML::Update(m_window, m_deltaClock.restart());
		handlePauseInput();
		if(!m_paused)
		{
			if (m_sLifespawn)
			{
				sLifespan();
			}
			if (m_sEnemySpawner)
			{
				sEnemySpawner();
			}
			if (m_sMovement)
			{
				sMovement();
			}
			if (m_sCollision)
			{
				sCollision();
			}
			if (m_sUserInput)
			{
				sUserInput();
			}
		}
		sGUI();
		sRender();

		m_currentFrame++;
	}
}
void Game::handlePauseInput()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
	{
		setPaused(m_paused);
		sf::sleep(sf::milliseconds(200));
	}
}
void Game::setPaused(bool paused)
{
	m_paused = !paused;
}

void Game::spawnPlayer()
{
	auto entity = m_entities.addEntity("player");
	entity->cTransform = std::make_shared<CTransform>(Vec2(m_wWidth / 2, m_wHeight / 2), Vec2(m_playerConfig.S, m_playerConfig.S), 0.0);
	entity->cInput = std::make_shared<CInput>();
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
	entity->cScore = std::make_shared <CScore>(0);
	m_player = entity;
}

void Game::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");
	Vec2 posTemp(std::rand() % (32 + (m_wWidth - m_wWidth/4)), std::rand() % ((m_wHeight - m_wWidth/4)));
	while (m_player->cTransform->pos == posTemp)
	{
		posTemp = Vec2(std::rand() % (32 + (m_wWidth - m_wWidth / 4)), std::rand() % (32 + (m_wWidth - m_wWidth / 4)));
	}
	int difSpeed = m_enemyConfig.SMAX - m_enemyConfig.SMIN;
	
	entity->cTransform = std::make_shared<CTransform>(posTemp, Vec2(m_enemyConfig.SMIN + std::rand() %  difSpeed, m_enemyConfig.SMIN + std::rand() % difSpeed), 0.0);
	

	entity->cShape = std::make_shared<CShape>(m_enemyConfig.CR, m_enemyConfig.VMIN + std::rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN),
		sf::Color(std::rand() % ((m_enemyConfig.OR - 1)), std::rand() % ((m_enemyConfig.OG - 1)), std::rand() % ((m_enemyConfig.OB - 1))),
		sf::Color(std::rand() % ((m_enemyConfig.OR - 1)), std::rand() % ((m_enemyConfig.OG - 1)), std::rand() % ((m_enemyConfig.OB - 1))), m_enemyConfig.OT);
	
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	int quantityOfSmallEnemies = e->cShape->circle.getPointCount();
	float angleStep = 2.0f * 3.14f / quantityOfSmallEnemies;
	float baseSpeed = 2.0f;

	for (int i = 0; i < quantityOfSmallEnemies; i++)
	{
		auto small = m_entities.addEntity("small");

		float posX = e->cShape->circle.getPoint(i).x + e->cTransform->pos.x;
		float posY = e->cShape->circle.getPoint(i).y + e->cTransform->pos.y;

		float angle = i * angleStep;
		float velocityX = baseSpeed * std::cos(angle);
		float velocityY = baseSpeed * std::sin(angle);

		small->cTransform = std::make_shared<CTransform>(Vec2(posX, posY), Vec2(velocityX, velocityY), 0.0);

		small->cShape = std::make_shared<CShape>(e->cShape->circle.getRadius() / 2, e->cShape->circle.getPointCount(), e->cShape->circle.getFillColor(), e->cShape->circle.getOutlineColor(),2.f);

		small->cCollision = std::make_shared<CCollision>(e->cCollision->radius / 2);
		small->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entities.addEntity("bullet");
	Vec2 pos(entity->cTransform->pos.x, entity->cTransform->pos.y);
	Vec2 d = target;
	d = pos.dist(d).normalize();
	bullet->cTransform = std::make_shared<CTransform>(pos, Vec2(m_bulletConfig.S * d.x, m_bulletConfig.S * d.y), 0.0);
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::sMovement()
{
	for (auto& e : m_entities.getEntities())
	{
		if (e == m_player)
		{
			continue;
		}
		if (e->cTransform->pos.x >= m_wWidth || e->cTransform->pos.x <= 0)
		{
			e->cTransform->velocity.x *= -1;
		}
		if (e->cTransform->pos.y >= m_wHeight || e->cTransform->pos.y <= 0)
		{
			e->cTransform->velocity.y *= -1;
		}
		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;
	}
	if (m_player->cInput->down)
	{
		m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
	}
	if (m_player->cInput->up)
	{
		m_player->cTransform->pos.y -= m_player->cTransform->velocity.y;
	}
	if (m_player->cInput->left)
	{
		m_player->cTransform->pos.x -= m_player->cTransform->velocity.x;
	}
	if (m_player->cInput->right)
	{
		m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
	}
}

void Game::sCollision()
{

	for (auto& b : m_entities.getEntities("bullet"))
	{
		for (auto& e : m_entities.getEntities("enemy"))
		{
			if (b->cTransform->pos.dist(e->cTransform->pos).distSq() < ((b->cCollision->radius + e->cCollision->radius) * (b->cCollision->radius + e->cCollision->radius)))
			{
				spawnSmallEnemies(e);
				b->destroy();
				e->destroy();
				m_player->cScore->score += 500;
			}
		}
		for (auto& s : m_entities.getEntities("small"))
		{
			if (b->cTransform->pos.dist(s->cTransform->pos).distSq() < ((b->cCollision->radius + s->cCollision->radius) * (b->cCollision->radius + s->cCollision->radius)))
			{
				b->destroy();
				s->destroy();
				m_player->cScore->score += 150;
			}
		}
	}
	for (auto& e : m_entities.getEntities("enemy"))
	{
		if (m_player->cTransform->pos.dist(e->cTransform->pos).distSq() < ((m_player->cCollision->radius + e->cCollision->radius) * (m_player->cCollision->radius + e->cCollision->radius)))
		{
			spawnSmallEnemies(e);
			m_player->destroy();
			e->destroy();
			spawnPlayer();
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnTime >= m_spawnTime)
	{
		spawnEnemy();
	}
}

void GuiHelp(EntityManager entities, const char* name)
{
	for (const auto& e : entities.getEntities(name))
	{
		ImGui::PushID(e->id());
		if (ImGui::ColorButton("##color", ImVec4(e->cShape->circle.getFillColor().r / 255.0f, e->cShape->circle.getFillColor().g / 255.0f, e->cShape->circle.getFillColor().b / 255.0f, e->cShape->circle.getFillColor().a / 255.0f), e->id()))
		{
			e->destroy();
		}

		ImGui::SameLine();
		ImGui::Text("%d %s (%.0f, %.0f)", e->id(), name, e->cTransform->pos.x, e->cTransform->pos.y);
		ImGui::PopID();
	}
}
void Game::sGUI()
{
	ImGui::Begin("Geometry wars");
	
	if (ImGui::BeginTabBar("Tabs")) 
	{
		if (ImGui::BeginTabItem("Systems"))
		{
			ImGui::Checkbox("Movement", &m_sMovement);
			ImGui::Checkbox("Lifespawn", &m_sLifespawn);
			ImGui::Checkbox("Collision", &m_sCollision);
			ImGui::Checkbox("SpawnEnemy", &m_sEnemySpawner);
			ImGui::SliderInt("Spawn Time", &m_spawnTime, 1, 200);
			if (ImGui::Button("SPAWN ENEMY!"))
			{
				spawnEnemy();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Entity Manager"))
		{
			if (ImGui::TreeNode("Entities by Tag")) 
			{
				if (ImGui::TreeNode("player")) 
				{

					for (const auto& e : m_entities.getEntities("player"))
					{
						ImGui::PushID(e->id());
						ImGui::ColorButton("##color", ImVec4(e->cShape->circle.getFillColor().r / 255.0f, e->cShape->circle.getFillColor().g / 255.0f, e->cShape->circle.getFillColor().b / 255.0f, e->cShape->circle.getFillColor().a / 255.0f), e->id());
						ImGui::SameLine();
						ImGui::Text("%d player (%.0f, %.0f)", e->id(), e->cTransform->pos.x, e->cTransform->pos.y);
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("bullet")) 
				{
					GuiHelp(m_entities, "bullet");
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("enemy")) 
				{
					GuiHelp(m_entities, "enemy");
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("small"))
				{
					GuiHelp(m_entities, "small");
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void Game::sLifespan()
{
	for (auto& e : m_entities.getEntities())
	{
		if (e->cLifespan == nullptr)
		{
			continue;
		}
		if (e->cLifespan->remaining > 0)
		{
			e->cLifespan->remaining -= 1;
			float lifePercentage = float(e->cLifespan->remaining) / e->cLifespan->total;
			sf::Color currentColor = e->cShape->circle.getFillColor();
			currentColor.a = (255 * lifePercentage);
			e->cShape->circle.setFillColor(currentColor);
			
		}
		if (e->cLifespan->remaining == 0)
		{
			e->destroy();
		}
	}
}

void Game::sRender()
{
	m_window.clear();

	m_text.setString("Score: " + std::to_string(m_player->cScore->score));
	m_window.draw(m_text);
	for (auto& e : m_entities.getEntities())
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		e->cTransform->angle += 1.f;
		e->cShape->circle.setRotation(e->cTransform->angle);
		m_window.draw(e->cShape->circle);
	}
	ImGui::SFML::Render(m_window);
	m_window.display();
}

void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(m_window, event);
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;
			case sf::Keyboard::Escape:
				m_window.close();
				exit(0);
				break;
			default:
				break;
			}
		}
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;
			default:
				break;
			}
		}
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				continue;
			}
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
		}
	}
}
