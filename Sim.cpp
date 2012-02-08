#include "Sim.hpp"
#include "SimMem.hpp"
#include "Graphics.hpp"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::make_pair;

namespace {
	class b2Vec2c : public b2Vec2 {
	public:
		b2Vec2c(Vector2f const &_in = make_vector(0.0f, 0.0f)) {
			*this = _in;
		}
		b2Vec2c &operator=(Vector2f const &b) {
			*static_cast< b2Vec2 * >(this) = b2Vec2(b.x, b.y);
			return *this;
		}
		b2Vec2c &operator=(Vector2d const &b) {
			*static_cast< b2Vec2 * >(this) = b2Vec2(b.x, b.y);
			return *this;
		}
	};

	void set(Vector2d &v, b2Vec2 const &b) {
		v.x = b.x;
		v.y = b.y;
	}
	void set(Vector2f &v, b2Vec2 const &b) {
		v.x = b.x;
		v.y = b.y;
	}
}

#define THREAD_LOCAL __thread

THREAD_LOCAL Sim *current_sim = NULL;

void *b2Alloc(int32 size) {
	assert(current_sim);
	return current_sim->b2Alloc(size);
}

void b2Free(void *mem) {
	assert(current_sim);
	current_sim->b2Free(mem);
}

//Helper to add static geometry to a box2d world given a list of points:
static b2Body *add_static_to_world(vector< Vector2f > const &verts, b2World *world) {
	Vector3f center = make_vector(0.0f, 0.0f, 0.0f);
	for (vector< Vector2f >::const_iterator p = verts.begin(); p != verts.end(); ++p) {
		center += make_vector(*p, 1.0f);
	}
	if (center.z != 0.0) {
		center.xy /= center.z;
	}
	vector< b2Vec2 > rel(verts.size());
	for (unsigned int i = 0; i < rel.size(); ++i) {
		rel[i] = b2Vec2(verts[i].x - center.x, verts[i].y - center.y);
	}

	b2BodyDef obj;
	obj.type = b2_staticBody;
	obj.position.Set(center.x, center.y);
	b2Body *body = world->CreateBody(&obj);
	b2ChainShape loop;
	loop.CreateLoop(&(rel[0]), rel.size());
	body->CreateFixture(&loop, 1.0f);
	return body;
}

Sim::Sim() : world(NULL), wheel(NULL), pin(NULL), last_block(NULL) {

	assert(current_sim == NULL);
	current_sim = this;

	{ //init world:
		b2Vec2 gravity(0.0f,-5.0f);
		//"placement new" so can checkpoint the world:
		world = new (b2Alloc(sizeof(b2World))) b2World(gravity);
	}

	//create terrain:
	{
		vector< Vector2f > plat;
		plat.push_back(make_vector(-7.0f,-1.0f));
		plat.push_back(make_vector(-7.0f, 0.5f));
		plat.push_back(make_vector(-5.0f, 0.5f));
		plat.push_back(make_vector(-5.0f, 0.0f));
		plat.push_back(make_vector(-3.0f,-0.1f));
		plat.push_back(make_vector(-1.0f, 0.1f));
		plat.push_back(make_vector( 1.0f, 0.6f));
		plat.push_back(make_vector( 3.0f, 0.1f));
		plat.push_back(make_vector( 5.0f, 0.2f));
		plat.push_back(make_vector( 7.0f, 0.4f));
		plat.push_back(make_vector( 7.0f,-1.0f));
		stuff.push_back(add_static_to_world(plat, world));
	}

	/*
	//create dynamic objects
	for (vector< DynamicItem >::const_iterator d = zone->dynamics.begin(); d != zone->dynamics.end(); ++d) {

		b2BodyDef obj;
		obj.type = b2_dynamicBody;
		obj.position.Set(d->pos.x, d->pos.y);
		obj.angle = d->pos.z;
		obj.allowSleep = true;
		if (d->gravity < Directions) {
			obj.gravity.Set(Gravity * Direction[d->gravity].x, Gravity * Direction[d->gravity].y);
		} else {
			obj.gravity.Set(Gravity * Direction[world_state.gravity_dir()].x, Gravity * Direction[world_state.gravity_dir()].y);
		}
		b2Body *body = world->CreateBody(&obj);

		for (vector< StaticGeom >::const_iterator c = d->shape.begin(); c != d->shape.end(); ++c) {
			if (c->type == StaticGeom::UNDEFINED_GEOM) continue;

			vector< b2Vec2 > rel(c->corners.size());
			for (unsigned int i = 0; i < rel.size(); ++i) {
				rel[i] = b2Vec2(c->corners[i].x - d->com.x, c->corners[i].y - d->com.y);
			}

			if (c->type == StaticGeom::LOOP_GEOM) {
				b2ChainShape loop;
				loop.CreateLoop(&(rel[0]), rel.size());
				body->CreateFixture(&loop, 0.0f);
			} else if (c->type == StaticGeom::CONVEX_GEOM) {
				b2PolygonShape convex;
				convex.Set(&(rel[0]), rel.size());
				body->CreateFixture(&convex, 0.0f);
			}
		}
		b2MassData mass;
		mass.mass = d->mass;
		mass.center.Set(0.0f, 0.0f); //shapes get com subtracted out, above.
		mass.I = d->moment;
		body->SetMassData(&mass);
		dynamics.push_back(body);
	}*/


	//create some small dynamic circles:
	for (unsigned int i = 0; i < 20; ++i) {
		//create body:
		b2BodyDef def;
		def.type = b2_dynamicBody;
		def.position.Set((i+0.5f) / 20.0f * 10.0f - 5.0f, 1.0f);
		b2Body *pebble = world->CreateBody(&def);
		
		//make circle:
		b2CircleShape wheel_shape;
		wheel_shape.m_radius = 0.05f + 0.1f * ((1 << i) % 7) / 7.0f;
		wheel_shape.m_p.Set(0.0f, 0.0f);
		b2FixtureDef fixture;
		fixture.shape = &wheel_shape;
		fixture.density = 0.1f;
		fixture.friction = 0.7f;
		fixture.restitution = 0.1f;
		pebble->CreateFixture(&fixture);
		stuff.push_back(pebble);
	}

	//create wheel
	{
		//create body:
		b2BodyDef wheel_def;
		wheel_def.type = b2_dynamicBody;
		wheel_def.position.Set(-6.0f, 1.1f);
		wheel = world->CreateBody(&wheel_def);
		
		//make circle:
		b2CircleShape wheel_shape;
		wheel_shape.m_radius = 0.5f;
		wheel_shape.m_p.Set(0.0f, 0.0f);
		b2FixtureDef fixture;
		fixture.shape = &wheel_shape;
		fixture.density = 0.1f;
		fixture.friction = 0.7f;
		fixture.restitution = 0.8f;
		wheel->CreateFixture(&fixture);

	}
	//create stick
	{
		b2BodyDef stick_def;
		stick_def.type = b2_dynamicBody;
		stick_def.position.Set(-6.0f, 1.1f);
		b2Body *stick = world->CreateBody(&stick_def);

		b2PolygonShape stick_shape;
		stick_shape.SetAsBox(1.0f, 0.1f);
		b2FixtureDef fixture;
		fixture.shape = &stick_shape;
		fixture.density = 0.1f;
		fixture.friction = 0.1f;
		fixture.restitution = 0.1f;
		stick->CreateFixture(&fixture);
		stuff.push_back(stick);

		b2RevoluteJointDef pin_def;
		pin_def.Initialize(wheel, stick, b2Vec2(-6.0f, 1.1f));
		pin_def.collideConnected = false;
		pin = dynamic_cast< b2RevoluteJoint * >(world->CreateJoint(&pin_def));
		assert(pin);

	}
}

Sim::~Sim() {

	world->~b2World();
	b2Free(world);

	if (last_block != NULL) {
		cerr << "Some block(s) b2Alloc'd didn't get b2Free'd:" << endl;
		BlockHeader *h = last_block;
		while (h != NULL) {
			cerr << "    Block of size " << h->size << " at " << h << endl;
			BlockHeader *old = h;
			h = h->prev;
			free(old);
		}
	}
	size_t total = 0;
	size_t blocks = 0;
	for (BlockMap::iterator f = free_lists.begin(); f != free_lists.end(); ++f) {
		while (f->second) {
			total += f->second->size;
			blocks += 1;
			BlockHeader *old = f->second;
			f->second = f->second->next;
			free(old);
		}
	}
	cout << "Sim used " << total << " bytes of memory in " << blocks << " blocks (not counting unfreed)." << endl;

	assert(current_sim == this);
	current_sim = NULL;
}


void Sim::tick(Control const &control) {

	if (control.signal == Control::LEFT) {
		pin->SetMotorSpeed(2.0f);
		pin->SetMaxMotorTorque(10.0f);
		pin->EnableMotor(true);
	} else if (control.signal == Control::RIGHT) {
		pin->SetMotorSpeed(-2.0);
		pin->SetMaxMotorTorque(10.0f);
		pin->EnableMotor(true);
	} else {
		assert(control.signal == Control::NONE);
		pin->EnableMotor(false);
	}

	assert(world->m_stackAllocator.m_index == 0);
	assert(world->m_stackAllocator.m_allocation == 0);
	world->Step(SimTick, 10, 10);
	assert(world->m_stackAllocator.m_index == 0);
	assert(world->m_stackAllocator.m_allocation == 0);

	++current_tick;
}

//helper for drawing box2d bodies:
static void draw_body(const b2Body *b) {
	glPushMatrix();

	{ //setup transform:
		b2Vec2 p = b->GetPosition();
		float ang = b->GetAngle();
		glTranslatef(p.x, p.y, 0.0);
		glRotatef(ang * (180.0f / float(M_PI)),  0.0, 0.0, 1.0);
	}

	for (const b2Fixture *fixture = b->GetFixtureList(); fixture != NULL; fixture = fixture->GetNext()) {
		const b2Shape *shape = fixture->GetShape();
		const b2ChainShape *chain = dynamic_cast< const b2ChainShape * >(shape);
		if (chain) {
			glBegin(GL_LINE_STRIP);
			for (unsigned int i = 0; i < unsigned(chain->m_count); ++i) {
				glVertex2f(chain->m_vertices[i].x, chain->m_vertices[i].y);
			}
			glEnd();
		}
		const b2CircleShape *circle = dynamic_cast< const b2CircleShape * >(shape);
		if (circle) {
			glBegin(GL_LINE_LOOP);
			glVertex2f(circle->m_p.x, circle->m_p.y);
			for (unsigned int a = 0; a < 64; ++a) {
				float ang = (a / 64.0f) * 2.0f * float(M_PI);
				glVertex2f(cosf(ang) * circle->m_radius + circle->m_p.x, sinf(ang) * circle->m_radius + circle->m_p.y);
			}
			glEnd();
		}
		const b2PolygonShape *poly = dynamic_cast< const b2PolygonShape * >(shape);
		if (poly) {
			glBegin(GL_LINE_LOOP);
			for (unsigned int i = 0; i < unsigned(poly->m_count); ++i) {
				glVertex2f(poly->m_vertices[i].x, poly->m_vertices[i].y);
			}
			glEnd();
		}
	}

	glPopMatrix();
}

void Sim::draw(float fade) const {
	for (vector< b2Body * >::const_iterator b = stuff.begin(); b != stuff.end(); ++b) {
		glColor4f(0.2f, 0.2f, 0.2f, fade);
		draw_body(*b);
	}

	glColor4f(0.7f, 0.0f, 0.0f, fade);
	draw_body(wheel);

}


typedef std::unordered_map< BlockHeader *, const StoredBlock * > FromMap;

void Sim::save(SimMem &into, SimMem *rel_mem) const {
	
	FromMap from_rel;
	if (rel_mem) {
		const StoredBlock *block = rel_mem->first_block;
		while (block) {
			from_rel.insert(make_pair(block->from, block));
			block = block->next;
		}
		into.rel = rel_mem;
	}

	into.state = *static_cast< const OtherState * >(this);
	into.free_blocks();
	assert(into.first_block == NULL);
	BlockHeader *to_store = last_block;
	while (to_store) {
		const StoredBlock *rel = NULL;
		{ //see if the relative memory happens to have this block:
			FromMap::iterator f = from_rel.find(to_store);
			if (f != from_rel.end()) {
				rel = f->second;
			}
		}
		StoredBlock *next = into.first_block;
		into.first_block = new StoredBlock(to_store, rel);
		into.first_block->next = next;
		to_store = to_store->prev;
	}
}

void Sim::load(SimMem const &from) {

	FromMap from_rel;
	if (from.rel) {
		const StoredBlock *block = from.rel->first_block;
		while (block) {
			from_rel.insert(make_pair(block->from, block));
			block = block->next;
		}
	}

	//Move allocated blocks to free lists again:
	while (last_block) {
		this->b2Free(last_block->mem());
	}

	*static_cast< OtherState * >(this) = from.state;

	const StoredBlock *to_load = from.first_block;
	while (to_load) {
		BlockHeader *to = to_load->from;
		assert(to);
		//Sanity check:
		assert(to->size == to_load->size);

		//Patch 'to' out of its free list:
		if (to->prev) {
			to->prev->next = to->next;
		}
		if (to->next) {
			to->next->prev = to->prev;
		}
		//Special handling if it happened to be the first in the list:
		if (to->prev == NULL) {
			BlockMap::iterator f = free_lists.find(to->size);
			assert(f != free_lists.end());
			assert(f->second == to);
			if (to->next) {
				f->second = to->next;
			} else {
				free_lists.erase(f);
			}
		}

		//Patch 'to' into our current blocks list:
		if (last_block) {
			assert(last_block->next == NULL);
			last_block->next = to;
		}
		to->prev = last_block;
		to->next = NULL;
		last_block = to;

		//Actually load contents:
		const StoredBlock *rel = NULL;
		{
			FromMap::iterator f = from_rel.find(to_load->from);
			if (f != from_rel.end()) {
				rel = f->second;
			}
		}
		to_load->set_block_mem(rel);

		//And move on...
		to_load = to_load->next;
	}
}

void *Sim::b2Alloc(int32 size) {
	BlockMap::iterator f = free_lists.find(size);
	BlockHeader *header = NULL;
	if (f != free_lists.end()) {
		//LOG_INFO("Allocating " << size << " grabs block " << f->second << ".");
		header = f->second;
		//Splice out of free list for size:
		f->second = header->next;
		if (f->second) {
			f->second->prev = NULL;
		} else {
			free_lists.erase(f);
		}
		//Clean up pointer into free list:
		header->next = NULL;
	} else {
		header = reinterpret_cast< BlockHeader * >(malloc(sizeof(BlockHeader) + size));
		//LOG_INFO("Allocating " << size << " makes new block " << header << ".");
		header->next = NULL;
		header->prev = NULL;
		header->size = size;
	}
	assert(header->next == NULL);
	assert(header->prev == NULL);
	assert(header->size == (size_t)size);

	//Zero memory -- might make compressing easier.
	memset(header->mem(), 0, header->size);

	if (last_block) {
		assert(last_block->next == NULL);
		last_block->next = header;
	}
	header->prev = last_block;
	last_block = header;
	return header->mem();
}

void Sim::b2Free(void *mem) {
	assert(mem);
	BlockHeader *block = BlockHeader::from_mem(mem);
	if (last_block == block) {
		assert(block->next == NULL);
		last_block = block->prev;
	}
	if (block->prev) {
		block->prev->next = block->next;
	}
	if (block->next) {
		block->next->prev = block->prev;
	}
	block->prev = NULL;
	block->next= NULL;

	//LOG_INFO("Hanging on to " << block << " of size " << block->size << ".");

	//possibly make a new (empty) free list for this size:
	BlockMap::iterator f = free_lists.insert(make_pair(block->size, reinterpret_cast< BlockHeader * >(NULL))).first;
	block->next = f->second;
	if (block->next) {
		block->next->prev = block;
	}
	f->second = block;
}

