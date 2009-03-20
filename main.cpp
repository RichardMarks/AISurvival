
// CODESTYLE: v2.0

// main.cpp
// Project: AI Survival Project ()
// Author: Richard Marks

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cctype>
#include <cstdarg>
#include <cstring>
#include <allegro.h>

#include <vector>

////////////////////////////////////////////////////////////////////////////////

/*
 
	I need to work in some kind of factor to slow down the commandos..right now they calculate their target vector and make a bee-line for it
	I need some kind of alert-zone for a targeted commando to "know" when he is about to get his ass whooped... XD

*/

////////////////////////////////////////////////////////////////////////////////

static volatile bool mainThreadIsRunning = true;
static volatile int allegroTimerSpeedCounter = 0;
static void AllegroCloseButtonHandler(){mainThreadIsRunning = false;}END_OF_FUNCTION(AllegroCloseButtonHandler)
static void AllegroTimerSpeedController(){allegroTimerSpeedCounter++;}END_OF_FUNCTION(AllegroTimerSpeedController)

////////////////////////////////////////////////////////////////////////////////

enum AIActionName
{
	AI_DoConvergence,
	AI_DoEvasion,
	AI_DoNothing
};

enum AISituationStateName
{
	/// the combat entity is idle, doing nothing at this time
	AI_Idle,
	
	/// the combat entity is patrolling the area
	AI_Patrolling,
	
	/// the combat entity is standing guard at his post and will not move unless provoked
	AI_Guarding,
	
	/// the combat entity is attacking its target
	AI_Attacking,
	
	/// the combat entity is chasing after its target
	AI_Chasing,
	
	/// the combat entity is evading; will run if any other entities get close
	AI_Evading,
	
	// the combat entity will move to a random position, memorizing the number of entities within his FOV
	// and return to his original position, alerting his team members with his report
	AI_Scouting,
	
	// the combat entity is fatigued, and needs to rest until his fatigue is gone
	AI_Resting
};
/*
	if the entity is idle and 
*/

////////////////////////////////////////////////////////////////////////////////

/*
	the combat entity has several properties that will determine its
	overall performance in combat.
	
	combatExperience_ is the number of targets that the combat entity has neutralized
	
	strenth_ is the amount of damage the combat entity can deal out
	
	health_ is the amount of damage that the combat entity can take before it is killed
	
	shield_ is the amount of damage that the combat entity can take before its health will be touched
	
	fatigue_ is the number of times the combat entity can move before needing to rest
	
	rangeOfSight_ is the distance from the combat entity that it can "see"
	
*/
class CombatEntityProperties
{
public:
	// constructor
	CombatEntityProperties()
	{
		this->Zero();
	}
	
	// destructor
	~CombatEntityProperties()
	{
		this->Zero();
	}

	// setters
	void SetCombatExperience(int amount = 0) 		{ combatExperience_ = amount; }
	void SetStrength(int amount = 0) 				{ strength_ = amount; }
	void SetHealth(int amount = 0) 					{ health_ = amount; }
	void SetShield(int amount = 0) 					{ shield_ = amount; }
	void SetFatigue(int amount = 0) 				{ fatigue_ = amount; }
	void SetRangeOfSight(int amount = 0) 			{ rangeOfSight_ = amount; }
	
	// incrementers
	void IncreaseCombatExperience(int amount = 1) 	{ combatExperience_ += amount; }
	void IncreaseStrength(int amount = 1) 			{ strength_ += amount; }
	void IncreaseHealth(int amount = 1) 			{ health_ += amount; }
	void IncreaseShield(int amount = 1) 			{ shield_ += amount; }
	void IncreaseFatigue(int amount = 1) 			{ fatigue_ += amount; }
	void IncreaseRangeOfSight(int amount = 1) 		{ rangeOfSight_ += amount; }
	
	// decrementers
	void DecreaseCombatExperience(int amount = 1) 	{ combatExperience_ -= amount; }
	void DecreaseStrength(int amount = 1) 			{ strength_ -= amount; }
	void DecreaseHealth(int amount = 1) 			{ health_ -= amount; }
	void DecreaseShield(int amount = 1) 			{ shield_ -= amount; }
	void DecreaseFatigue(int amount = 1) 			{ fatigue_ -= amount; }
	void DecreaseRangeOfSight(int amount = 1) 		{ rangeOfSight_ -= amount; }
	
	// getters
	int GetCombatExperience() const 				{ return combatExperience_; }
	int GetStrength() const 						{ return strength_; }
	int GetHealth() const 							{ return health_; }
	int GetShield() const 							{ return shield_; }
	int GetFatigue() const							{ return fatigue_; }
	int GetRangeOfSight() const						{ return rangeOfSight_; }
	
	// adds the inheritable properties of the parent to this entity
	void InheritProperties(const CombatEntityProperties& parent)
	{
		this->IncreaseStrength(parent.strength_);
		this->IncreaseRangeOfSight(parent.rangeOfSight_);
	}
	
	// clones the properties of the source entity to this entity
	void Clone(const CombatEntityProperties& source)
	{
		this->SetCombatExperience(source.combatExperience_);
		this->SetStrength(source.strength_);
		this->SetHealth(source.health_);
		this->SetShield(source.shield_);
		this->SetFatigue(source.fatigue_);
		this->SetRangeOfSight(source.rangeOfSight_);
	}
	
	// clears all properties to the lowest values
	void Zero()
	{
		this->SetCombatExperience();
		this->SetStrength();
		this->SetHealth();
		this->SetShield();
		this->SetFatigue();
		this->SetRangeOfSight();
	}
	
private:
	int combatExperience_;
	int strength_;
	int health_;
	int shield_;
	int fatigue_;
	int rangeOfSight_;
}; // end class

////////////////////////////////////////////////////////////////////////////////

class CombatEntity
{
public:
	CombatEntity() :
	
		color_(0),
		x_(0),
		y_(0),
		width_(8),
		height_(16),

		ai_(AI_DoNothing),		
		isTargeted_(false),
		isNeutralized_(false),
		isWithinRangeOfTargetEntity_(false),
		targetEntity_(0)
		
	{
	}
	
	virtual ~CombatEntity()
	{
	}
	
	virtual void Update() = 0;
	virtual void Render(BITMAP* target) = 0;
	
	
	
	void SetColor(int color) { color_ = color; }
	void SetPosition(int x, int y) { x_ = x; y_ = y; }
	void SetSize(int width, int height) { width_ = width; height_ = height; }
	int GetColor() const { return color_; }
	int GetX() const { return x_; }
	int GetY() const { return y_; }
	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	
	void SetAIAction(AIActionName action) { ai_ = action; }
	AIActionName GetAIAction() const { return ai_; }
	
	CombatEntityProperties& GetProperties() { return properties_; }
	
	virtual void Target(bool targeted = true) { isTargeted_ = targeted; }
	bool IsTargeted() const { return isTargeted_; }
	
	virtual void Neutralize(bool neutralize = true) { isNeutralized_ = neutralize; }
	bool IsNeutralized() const { return isNeutralized_; }
	
	virtual void WithinRangeOfTargetEntity(bool inRange = true) { isWithinRangeOfTargetEntity_ = inRange; }
	bool IsWithinRangeOfTargetEntity() const { return isWithinRangeOfTargetEntity_; }
	
	virtual void SetTargetEntity(CombatEntity* targetEntity) { targetEntity_ = targetEntity; }
	CombatEntity* GetTargetEntity() const { return targetEntity_; }
	
protected:
	// positioning and rendering
	int color_;
	int x_;
	int y_;
	int width_;
	int height_;

protected:
	// AI
	AIActionName ai_;
	CombatEntityProperties properties_;
	
	bool isTargeted_;
	bool isNeutralized_;
	bool isWithinRangeOfTargetEntity_;
	CombatEntity* targetEntity_;
}; // end class

////////////////////////////////////////////////////////////////////////////////

class CombatEntityList
{
public:
	CombatEntityList();
	~CombatEntityList();
	void Push(CombatEntity* entity);
	CombatEntity* operator[](size_t index);
	size_t Size() const;
	void Erase(size_t index);
private:
	std::vector<CombatEntity*> list_;
}; // end class

////////////////////////////////////////////////////////////////////////////////

CombatEntityList::CombatEntityList()
{
}

////////////////////////////////////////////////////////////////////////////////

CombatEntityList::~CombatEntityList()
{
	size_t count = this->Size();
	
	for (size_t index = 0; index < count; index++)
	{
		if (list_[index])
		{
			delete list_[index];
			list_[index] = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void CombatEntityList::Push(CombatEntity* entity)
{
	list_.push_back(entity);
}

////////////////////////////////////////////////////////////////////////////////

CombatEntity* CombatEntityList::operator[](size_t index)
{
	return (index < this->Size()) ? list_.at(index) : 0;
}

////////////////////////////////////////////////////////////////////////////////

size_t CombatEntityList::Size() const
{
	return list_.size();
}

////////////////////////////////////////////////////////////////////////////////

void CombatEntityList::Erase(size_t index)
{
	if (list_[index])
	{
		delete list_[index];
		list_[index] = 0;
	}
	list_.erase(list_.begin() + index);
}

////////////////////////////////////////////////////////////////////////////////

class Commando : public CombatEntity
{
public:
	Commando();
	Commando(int x, int y);
	virtual ~Commando();
	virtual void Update();
	virtual void Render(BITMAP* target);
	virtual void Target(bool targeted = true);
	
	virtual void SetTargetEntity(CombatEntity* targetEntity);
private:
	int aiTargetX_;
	int aiTargetY_;
	bool reachedAITarget_;
}; // end class

////////////////////////////////////////////////////////////////////////////////

Commando::Commando()
{
	this->SetColor(makecol(0, 255, 0));
	reachedAITarget_ = false;
}

////////////////////////////////////////////////////////////////////////////////

Commando::Commando(int x, int y)
{
	this->SetPosition(x, y);
	this->SetColor(makecol(0, 255, 0));
	reachedAITarget_ = false;
}

////////////////////////////////////////////////////////////////////////////////

Commando::~Commando()
{
}

////////////////////////////////////////////////////////////////////////////////

void Commando::Update()
{
	// is this commando dying?
	if (properties_.GetHealth() <= 0)
	{
		// its dead
		this->Neutralize();
		return;
	}
	
	
	
	// handle the ai
	switch(ai_)
	{
		case AI_DoEvasion:
		{
			// if the commando has not reached its target
			if (!reachedAITarget_)
			{
				// if the commando is not close enough to the target
				if ((abs(x_ - aiTargetX_) + abs(y_ - aiTargetY_)) > (height_ + width_))
				{
					// compute deltas
					// compute unit vector to centroid
					// scale unit vector for synthesis
					// compute trajectory vector
					// apply velocity to position
					float dx = static_cast<float>(aiTargetX_) - static_cast<float>(x_);
					float dy = static_cast<float>(aiTargetY_) - static_cast<float>(y_);
					float length = sqrt(dx * dx + dy * dy);
			
					dx /= length;
					dy /= length;
					int speed = 1 + rand() % (2 - 1);
					float velocityX = dx * speed;
					float velocityY = dy * speed;
					float x = static_cast<float>(x_);
					float y = static_cast<float>(y_);
				
					x += velocityX;
					y += velocityY;
					x_ = static_cast<int>(x);
					y_ = static_cast<int>(y);
				}
				else
				{
					// pick a random point and head that way
					aiTargetX_ = rand() % SCREEN_W;
					aiTargetY_ = rand() % SCREEN_H;
				}
			}
		} break;
		
		case AI_DoConvergence:
		{
			aiTargetX_ = targetEntity_->GetX();
			aiTargetY_ = targetEntity_->GetY();
			
			// if the commando has not reached its target
			if (!reachedAITarget_)
			{
				// if the commando is not close enough to the target
				if ((abs(x_ - aiTargetX_) + abs(y_ - aiTargetY_)) > (height_ + width_))
				{
					// compute deltas
					// compute unit vector to centroid
					// scale unit vector for synthesis
					// compute trajectory vector
					// apply velocity to position
					float dx = static_cast<float>(aiTargetX_) - static_cast<float>(x_);
					float dy = static_cast<float>(aiTargetY_) - static_cast<float>(y_);
					float length = sqrt(dx * dx + dy * dy);
			
					dx /= length;
					dy /= length;
					int speed = 1 + rand() % (2 - 1);
					float velocityX = dx * speed;
					float velocityY = dy * speed;
					float x = static_cast<float>(x_);
					float y = static_cast<float>(y_);
				
					x += velocityX;
					y += velocityY;
					x_ = static_cast<int>(x);
					y_ = static_cast<int>(y);
				}
				else
				{
					reachedAITarget_ = true;
					
					
					// how much are we going to damage the target entity
					int damage = properties_.GetStrength();
					
					// target properties
					CombatEntityProperties& targetProperties = targetEntity_->GetProperties();
					
					// get the target's shield
					int targetShield = targetProperties.GetShield();
					
					// calculate if we will demolish the target's shield
					int overflow = abs(targetShield - damage);
					
					// if we have wiped out the shields
					if (overflow)
					{
						// make sure its zero
						targetProperties.SetShield(0);
						
						// damage the health of the target
						targetProperties.DecreaseHealth(overflow);
					}
					else
					{
						// damage the shields of the target
						targetProperties.DecreaseShield(damage);
					}
					
					// get the target's health
					int targetHealth = targetProperties.GetHealth();
					
					// if the target's health is critical
					if (targetHealth <= 0)
					{
						// we kill it off
						targetProperties.SetHealth(0);
						targetEntity_->Neutralize();
						
						// advance this commando
						properties_.IncreaseCombatExperience();
						properties_.IncreaseStrength();
						properties_.IncreaseHealth();
						properties_.IncreaseShield();
					}
					
					// tell the commando to do nothing now
					this->SetAIAction(AI_DoNothing);
				}
			}
			
		} break;
		
		default: break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Commando::Render(BITMAP* target)
{
	int wOver2 = width_ / 2;
	int hOver2 = height_ / 2;
	
	if (AI_DoConvergence == ai_ && !reachedAITarget_)
	{
		line(target, x_ + wOver2, y_ + hOver2, aiTargetX_ + wOver2, aiTargetY_ + hOver2, makecol(64, 0, 0));
	}
	
	rectfill(target, x_, y_, x_ + width_, y_ + height_, color_);
	
	circle(target, x_ + wOver2, y_ + hOver2, properties_.GetRangeOfSight(), makecol(0, 64, 0));	
	circle(target, x_ + wOver2, y_ + hOver2, properties_.GetFatigue(), makecol(64, 0, 0));	
}

////////////////////////////////////////////////////////////////////////////////

void Commando::Target(bool targeted)
{
	CombatEntity::Target(targeted);
	
	if (isTargeted_)
	{
		this->SetColor(makecol(255, 255, 0));
		this->SetAIAction(AI_DoEvasion);
		
		// pick a random point and head that way
		aiTargetX_ = rand() % SCREEN_W;
		aiTargetY_ = rand() % SCREEN_H;
	}
	else
	{
		this->SetColor(makecol(0, 255, 0));
	}
}

void Commando::SetTargetEntity(CombatEntity* targetEntity)
{
	CombatEntity::SetTargetEntity(targetEntity);
	aiTargetX_ = targetEntity->GetX();
	aiTargetY_ = targetEntity->GetY();
}

////////////////////////////////////////////////////////////////////////////////

class Simulator
{
public:
	Simulator();
	~Simulator();
	bool Initialize();
	void Update();
	void Render(BITMAP* target);
	void Destroy();
	
private:
	// simulator "engine" members
	bool isSimulationRunning_;
	bool lmbIsDown_;
	bool rmbIsDown_;
	bool keyDown_[KEY_MAX];
	
private:
	// simulation object members
	CombatEntityList combatEntities_;
	
}; // end class

////////////////////////////////////////////////////////////////////////////////

Simulator::Simulator() :
	isSimulationRunning_(false),
	lmbIsDown_(false),
	rmbIsDown_(false)
{
	for (int index = 0; index < KEY_MAX; index++)
	{
		keyDown_[index] = false;
	}
}

////////////////////////////////////////////////////////////////////////////////

Simulator::~Simulator()
{
	this->Destroy();
}

////////////////////////////////////////////////////////////////////////////////

bool Simulator::Initialize()
{

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Simulator::Update()
{
	if (mouse_b & 1)
	{
		if (!lmbIsDown_)
		{
			lmbIsDown_ = true;
		}
	}
	else
	{
		if (lmbIsDown_)
		{
			// click
			Commando* nextCommando = new Commando(mouse_x, mouse_y);
			CombatEntityProperties rollOfTheDice;
			rollOfTheDice.SetCombatExperience(0);
			rollOfTheDice.SetStrength(10 + rand() % (20 - 10));
			rollOfTheDice.SetHealth(100 + rand() % (200 - 100));
			rollOfTheDice.SetShield(100 + rand() % (200 - 100));
			rollOfTheDice.SetFatigue(32 + rand() % (96 - 32));
			rollOfTheDice.SetRangeOfSight((SCREEN_H / 8) + rand() % ((SCREEN_H / 5) - (SCREEN_H / 8)));
			nextCommando->GetProperties().Clone(rollOfTheDice);
			combatEntities_.Push(nextCommando);
			
			lmbIsDown_ = false;
		}
	}
	
	if (mouse_b & 2)
	{
		if (!rmbIsDown_)
		{
			rmbIsDown_ = true;
		}
	}
	else
	{
		if (rmbIsDown_)
		{
			rmbIsDown_ = false;
			// do something when the right mouse button is clicked
		}
	}
	
	// pick convergence simulation
	if (key[KEY_1])
	{
		if (!keyDown_[KEY_1])
		{
			keyDown_[KEY_1] = true;
		}
	}
	else
	{
		if (keyDown_[KEY_1])
		{
			keyDown_[KEY_1] = false;
			
			// if the simulation is running, then we cannot interrupt it
			if (!isSimulationRunning_)
			{
				// get the number of commandos
				size_t commandoCount = combatEntities_.Size();
				
				// pick a random commando to target
				size_t targetCommandoIndex = rand() % static_cast<int>(commandoCount - 1);
				
				// we tell all the commandos to enter the convergence mode and its target
				for (size_t index = 0; index < commandoCount; index++)
				{
					// we skip over the target commando
					if (targetCommandoIndex == index)
					{
						continue;
					}
					
					// enter convergence mode
					combatEntities_[index]->SetAIAction(AI_DoConvergence);
					
					// set the target of the commando
					combatEntities_[index]->SetTargetEntity(combatEntities_[targetCommandoIndex]);
				}
				
				// tell the target commando that its a target
				combatEntities_[targetCommandoIndex]->Target();
			
			}
		}
	}
	
	// toggle simulation runtime
	if (key[KEY_R])
	{
		if (!keyDown_[KEY_R])
		{
			keyDown_[KEY_R] = true;
		}
	}
	else
	{
		if (keyDown_[KEY_R])
		{
			keyDown_[KEY_R] = false;
			
			isSimulationRunning_ = !isSimulationRunning_;
		}
	}
	
	
	
	// only update the simulation if we are running
	if (isSimulationRunning_)
	{
		size_t commandoCount = combatEntities_.Size();
		for (size_t index = 0; index < commandoCount; index++)
		{
			CombatEntity* entity = combatEntities_[index];
			if (entity)
			{
				// is this entity dead?
				if (entity->IsNeutralized())
				{
					// remove the dead entity from the field
					combatEntities_.Erase(index);
					
					// update the count
					commandoCount = combatEntities_.Size();
					continue;
				}
			
				// update the entity
				entity->Update();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void Simulator::Render(BITMAP* target)
{
	static int hudOriginX = 4;
	static int hudOriginY = 4;
	int hudX = hudOriginX;
	int hudY = hudOriginY;
	int txtH = text_height(font);
	int txtC = makecol(192, 192, 192);
	
	// get the number of commandos
	size_t commandoCount = combatEntities_.Size();
	
	// if there are more than 0
	if (commandoCount)
	{
		// for each commando
		for (size_t index = 0; index < commandoCount; index++)
		{
			// grab a pointer to the commando
			CombatEntity* commando = combatEntities_[index];
			
			// grab a pointer to the target commando
			CombatEntity* targetCommando = commando->GetTargetEntity();
			
			// get the ai action of the commando
			AIActionName ai = commando->GetAIAction();
			
			// get the properties of the commando
			CombatEntityProperties& properties = commando->GetProperties();
		
			
			// create the hud information
			char hudLines[4][0x80];
			sprintf(hudLines[0], "Commando %3d @ Pos(%4d, %4d)", static_cast<int>(index) + 1, commando->GetX(), commando->GetY());
			sprintf(hudLines[1], "AI Action: %s", (AI_DoNothing == ai) ? "Do Nothing" : (AI_DoConvergence) ? "Converge on Target" : "Take Evasive Action");
			sprintf(hudLines[2], "HP: %4d    SH: %4d    STR: %4d    XP: %6d", properties.GetHealth(), properties.GetShield(), properties.GetStrength(), properties.GetCombatExperience());
			sprintf(hudLines[3], "Has Target? %s  Pos(%4d, %4d)", (targetCommando) ? "YES" : "NO", (targetCommando) ? targetCommando->GetX() : 0, (targetCommando)?targetCommando->GetY() : 0);
			//sprintf(hudLines[4], "AI State: %s");
			
			// render the commando
			combatEntities_[index]->Render(target);

			// render the hud information
			textprintf_ex(target, font, hudX, hudY, txtC, -1, "%s", hudLines[0]);
			textprintf_ex(target, font, hudX, hudY + (txtH + 2), txtC, -1, "%s", hudLines[1]);
			textprintf_ex(target, font, hudX, hudY + ((txtH * 2) + 4), txtC, -1, "%s", hudLines[2]);
			textprintf_ex(target, font, hudX, hudY + ((txtH * 3) + 6), txtC, -1, "%s", hudLines[3]);
			
			// update the hude info position
			hudY += (txtH * 5) + 10;
			/*
			textprintf_ex(target, font, hudX, hudY + (txtH * index) + (2 * index), txtC, -1,
				"Commando %3d : Pos(%4d, %4d) Orders: %s [HP: %4d SH: %4d STR: %4d XP: %d]", 
				static_cast<int>(index) + 1, commando->GetX(), commando->GetY(),
				(AI_DoNothing == ai) ? "Do Nothing" : "Converge on Target",
				properties.GetHealth(), properties.GetShield(), properties.GetStrength(), properties.GetCombatExperience());
			*/
		}
	}
	
	textprintf_ex(target, font, hudOriginX, target->h - txtH, txtC, -1, "Commandos: %d", static_cast<int>(commandoCount));
}

////////////////////////////////////////////////////////////////////////////////

void Simulator::Destroy()
{
}


////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	allegro_init();
	install_keyboard();
	install_timer();
	install_mouse();
	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1024, 800, 0, 0);
	set_window_title("AI Survival v0.2 - Developed by Richard Marks <ccpsceo@gmail.com>");
	BITMAP* bb = create_bitmap(SCREEN_W, SCREEN_H);
	LOCK_FUNCTION(AllegroCloseButtonHandler);
	LOCK_FUNCTION(AllegroTimerSpeedController);
	LOCK_VARIABLE(allegroTimerSpeedCounter);
	
	set_close_button_callback(AllegroCloseButtonHandler);
	install_int_ex(AllegroTimerSpeedController, BPS_TO_TIMER(60));
	
	Simulator simulator;
	
	if (!simulator.Initialize())
	{
		fprintf(stderr, "Simulator Initialization Failed!\n");
		show_mouse(0);
		destroy_bitmap(bb);
		return 1;
	}
	
	while(mainThreadIsRunning)
	{
		if (keyboard_needs_poll())
		{
			poll_keyboard();
		}
		
		if (mouse_needs_poll())
		{
			poll_mouse();
		}
		
		if (key[KEY_ESC])
		{
			mainThreadIsRunning = false;
		}
		
		while (allegroTimerSpeedCounter > 0)
		{
			// update everything here
			simulator.Update();
			
			allegroTimerSpeedCounter--;
		}
		
		clear_bitmap(bb);
		
		// render everything here
		simulator.Render(bb);
		
		show_mouse(bb);
		blit(bb, screen, 0, 0, 0, 0, bb->w, bb->h);
		rest(10);
	}
	show_mouse(0);
	destroy_bitmap(bb);
	
	simulator.Destroy();
	
	return 0;
}
END_OF_MAIN()


