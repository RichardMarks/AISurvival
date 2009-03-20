
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

static volatile bool mainThreadIsRunning = true;
static volatile int allegroTimerSpeedCounter = 0;
static void AllegroCloseButtonHandler(){mainThreadIsRunning = false;}END_OF_FUNCTION(AllegroCloseButtonHandler)
static void AllegroTimerSpeedController(){allegroTimerSpeedCounter++;}END_OF_FUNCTION(AllegroTimerSpeedController)

////////////////////////////////////////////////////////////////////////////////

enum AIActionName
{
	AI_DoConvergence,
	AI_DoNothing
};

////////////////////////////////////////////////////////////////////////////////

class CombatEntity
{
public:
	CombatEntity() :
		color_(0),
		x_(0),
		y_(0),
		width_(8),
		height_(8),
		isTargeted_(false),
		ai_(AI_DoNothing)
		{}
	virtual ~CombatEntity(){}
	virtual void Update() = 0;
	virtual void Render(BITMAP* target) = 0;
	
	virtual void AIAction(AIActionName action, int x, int y) = 0;
	
	virtual void Target(bool targeted = true) { isTargeted_ = targeted; }
	bool IsTargeted() const { return isTargeted_; }
	
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
	
protected:
	int color_;
	int x_;
	int y_;
	int width_;
	int height_;
	bool isTargeted_;
	AIActionName ai_;
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

class Commando : public CombatEntity
{
public:
	Commando();
	Commando(int x, int y);
	virtual ~Commando();
	virtual void Update();
	virtual void Render(BITMAP* target);
	virtual void Target(bool targeted = true);
	virtual void AIAction(AIActionName action, int x, int y);
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
	switch(ai_)
	{
		case AI_DoConvergence:
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
					reachedAITarget_ = true;
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
	if (AI_DoConvergence == ai_ && !reachedAITarget_)
	{
		int wOver2 = width_ / 2;
		int hOver2 = height_ / 2;
		line(target, x_ + wOver2, y_ + hOver2, aiTargetX_ + wOver2, aiTargetY_ + hOver2, makecol(64, 0, 0));
	}
	
	rectfill(target, x_, y_, x_ + width_, y_ + height_, color_);	
}

////////////////////////////////////////////////////////////////////////////////

void Commando::Target(bool targeted)
{
	CombatEntity::Target(targeted);
	
	if (isTargeted_)
	{
		this->SetColor(makecol(255, 255, 0));
	}
	else
	{
		this->SetColor(makecol(0, 255, 0));
	}
}

////////////////////////////////////////////////////////////////////////////////

void Commando::AIAction(AIActionName action, int x, int y)
{
	this->SetAIAction(action);
	
	aiTargetX_ = x;
	aiTargetY_ = y;
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
	CombatEntityList combatEntities_;
	bool lmbIsDown_;
	bool rmbIsDown_;
}; // end class

////////////////////////////////////////////////////////////////////////////////

Simulator::Simulator() :
	lmbIsDown_(false),
	rmbIsDown_(false)
{
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
			combatEntities_.Push(new Commando(mouse_x, mouse_y));
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
			// click
			
			// pick a random commando to target
			size_t commando = rand() % static_cast<int>(combatEntities_.Size() - 1);
			combatEntities_[commando]->Target();
			
			// alert all other commandos about the target
			size_t count = combatEntities_.Size();
			for (size_t index = 0; index < count; index++)
			{
				if (index == commando)
				{
					continue;
				}
				combatEntities_[index]->AIAction(
					AI_DoConvergence,
					combatEntities_[commando]->GetX(), 
					combatEntities_[commando]->GetY());
			}
			
			rmbIsDown_ = false;
		}
	}
	
	


	size_t count = combatEntities_.Size();
	for (size_t index = 0; index < count; index++)
	{
		combatEntities_[index]->Update();
	}
}

////////////////////////////////////////////////////////////////////////////////

void Simulator::Render(BITMAP* target)
{
	int hudX = 4;
	int hudY = 4;
	int txtH = text_height(font);
	int txtC = makecol(255, 255, 255);
	
	size_t count = combatEntities_.Size();
	for (size_t index = 0; index < count; index++)
	{
		CombatEntity* commando = combatEntities_[index];
		AIActionName ai = commando->GetAIAction();
		
		textprintf_ex(target, font, hudX, hudY + (txtH * index) + (2 * index), txtC, -1,
		"Commando %d : Pos(%d, %d) Orders: %s", static_cast<int>(index) + 1, commando->GetX(), commando->GetY(),
			(AI_DoNothing == ai) ? "Do Nothing" : "Converge on Target");
		
		combatEntities_[index]->Render(target);
	}
	
	textprintf_ex(target, font, hudX, target->h - (hudY + txtH * 2), txtC, -1, "Commandos: %d", static_cast<int>(count));
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
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0);
	set_window_title("AI Survival v0.1 - Developed by Richard Marks <ccpsceo@gmail.com>");
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


