#include "Entity.h"

Entity::Entity(const size_t dataId, const std::string& dataTag)
	: m_id(dataId), m_tag(dataTag)
{
}

bool Entity::isAlive() const
{
	return m_active;
}

const std::string& Entity::tag() const
{
	return m_tag;
}

const size_t Entity::id() const
{
	return m_id;
}

void Entity::destroy()
{
	m_active = false;
}
