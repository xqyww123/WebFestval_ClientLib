/* I do not know what to write here...
* So leave it blank~!
* Write by Xero Essential!~
*
* */
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_set>
#include <array>
#include <iostream>
#define CURRENT_ROUND -1
#define LAST_ROUND -2
#define NEXT_ROUND -3
#define CARD_NUM 104
#define FIRST_ROUND 1
#define CARD_UNKNOWN -1
#define CARD_DROPED -2
using namespace std;
typedef int Card;

class Agent;

class AgentRef
{
	string _name;
	int _index;
	int _score;
public:
	friend class Agent;
	AgentRef(string name, int index) : _name(name), _index(index), _score(0) {}
	AgentRef(istream& in, int index) : _index(index), _score(0) { in >> _name; }
	const string& name() const { return _name; }
	int index() const { return _index; }
	int score() const { return _score; }
};

class Cluster : public vector<Card>
{
public:
	Cluster() {}
};

template <typename T>
class AgentDic : public vector<T>
{
public:
	AgentDic() : vector() {}
	AgentDic(int num) : vector(num) {}
	const T& operator[] (AgentRef& agent) const { return vector<T>::operator [] (agent.index()); }
	T& operator[] (AgentRef& agent) { return vector<T>::operator [] (agent.index()); }
};

class PunishInfo
{
	AgentRef& _who;
	Cluster& _which;
public:
	PunishInfo(AgentRef& who, Cluster& which) : _who(who), _which(which) {}
	const AgentRef& who() const { return _who; }
	const Cluster which() const { return _which; }
};

class Agent
{
	Card _card_decision;
	Cluster* _eat_decision;
	void clear_last_decision();

	vector<AgentRef*> _players;
	vector<unordered_set<Card> > _mycards;
	vector<int> _droped_cards;
	vector<array<Cluster, 4> > _cards;
	vector<AgentDic<int> > _showed_cards;
	vector<PunishInfo> _punish_infos;
	array<AgentRef*, CARD_NUM + 1> _whouse;
	int curi, _myindex;
	static int _score_table[CARD_NUM + 1];
	static void init_score_table();

	void init();
protected:
	Agent() { init_score_table(); }
	const int trans_index(int round_index) const;
	int calcu_score(const vector<Card>& cards) const;
	int get_score(Card) const;
public:

	// please override this function and return your contest id;
	AgentRef& me() { return *_players[_myindex]; }

	const vector<AgentRef*>& players() const { return _players; }
	int players_count() const { return _players.size(); }

	int current_turn_index() const { return curi; }
	const array<Cluster, 4>& cards(int round_index = CURRENT_ROUND) const { return _cards[trans_index(round_index)]; }
	const unordered_set<Card>& my_cards(int round_index = CURRENT_ROUND) const { return _mycards[trans_index(round_index)]; }
	const AgentDic<Card>& used_card(int round_index = LAST_ROUND) const { return _showed_cards[trans_index(round_index)]; }
	const PunishInfo& punish_infos() const {}

	// some card may droped before game start
	vector<Card>& droped_crads() { return _droped_cards; }
	// a vector shows each card was used by who
	// NULL means unknown or droped
	const array<AgentRef*, CARD_NUM + 1> who_use() const { return _whouse; }

	// decide the card to use in current turn.
	void decide_card(const Card card) { _card_decision = card; }
	// decide the cluster to eat.
	void decide_eat(const Cluster& cluster);

	virtual void on_game_start() {}
	virtual void on_round_start(int round_index) {}
	// In this process : please make your decision with decide_card function.
	// If your didn't call decide_card function, the program will throw an exception.
	// The latest call of decide_card fucntion will be adopt for the multiple calls.
	virtual void on_my_turn(int round_index) = 0;
	// please call decide_eat function before the on_eat_cluster process end.
	// you could call decide_eat in either on_eat_cluster fucntion or any other place such as on_round_start function
	// ( It's your right to consider the bad situation early, of course. )
	// But if you forget to call decide_eat function, an exception will be throwed.
	// The latest call of decide_eat fucntion will be adopt for the multiple calls.
	virtual void on_eat_cluster(int round_index) = 0;
	virtual void on_round_end(int round_index) {}
	virtual void on_game_end() {}

	void Run();
};

int Agent::_score_table[CARD_NUM + 1];

void Agent::clear_last_decision()
{
	_card_decision = -1;
	_eat_decision = NULL;
}

char scan_buf[256];
void Agent::init()
{
	_showed_cards.clear();
	_showed_cards.push_back(AgentDic<int>(0));
	_whouse.fill(NULL);

	curi = 0;
	int n, m, p, a;
	cin >> n >> m >> p;
	for (auto a : _players) delete a;
	_players.clear();
	for (int i = 0; i < n; ++i)
		_players.push_back(new AgentRef(cin, i));
	_players.push_back(new AgentRef(string("me"), n));
	_myindex = n;
	_mycards.clear();
	_mycards.push_back(unordered_set<int>());
	for (int i = 0; i < m; ++i)
		{ cin >> a; _mycards[0].insert(a); }
	for (int i = 0; i < p; ++i)
	{ cin >> a; _droped_cards.push_back(a); _whouse[a] = NULL; }
	_cards.clear();
	_cards.push_back(array<Cluster, 4>());
	for (int j = 0; j < 4; ++j)
	{
		cin >> a; _cards[0][j].push_back(a);
	}

}

void Agent::Run()
{
	init();
	on_game_start();
	int a;
	int* inp = new int[players_count()];
	while (true)
	{
		clear_last_decision();
		on_round_start(curi);
		on_my_turn(curi);
		if (_card_decision == -1)
			throw "ERR01 You forget make card decision";
		cout << (inp[players_count()-1] = _card_decision) << endl;
		_showed_cards.push_back(AgentDic<int>(players_count()));
		_showed_cards[curi+1][me()] = _card_decision;
		_whouse[_card_decision] = &me();
		_mycards.push_back(_mycards[curi]);
		_mycards[curi + 1].erase(_card_decision);
		/* ==== After Decision ==== */
		for (int i = 0; i < players_count() - 1; ++i)
		{
			cin >> inp[i];
			_showed_cards[curi+1][*players()[i]] = inp[i];
			_whouse[inp[i]] = players()[i];
		}
		_cards.push_back(array<Cluster, 4>());
		for (int i = 0; i < 4; ++i) _cards[curi + 1][i] = _cards[curi][i];
		for (int i = 0; i < players_count(); ++i)
		{
			int vmin = INT_MAX, vmi;
			for (int j = 0; j<players_count(); ++j)
				if (vmin > inp[j])
					{ vmin = inp[j]; vmi = j; }
			inp[vmi] = INT_MAX;
			int ci, cv = -1;
			for (int j = 0; j < 4; ++j)
			if (cards(NEXT_ROUND)[j].back() < vmin && cards(NEXT_ROUND)[j].back() > cv)
				{ cv = cards(NEXT_ROUND)[j].back(); ci = j; }
			if (cv == -1)
			{
				Cluster* eat;
				if (vmi == _myindex)
				{
					on_eat_cluster(curi);
					if (_eat_decision == NULL)
						throw "ERR02 You forget make eat decision!";
					cout << (eat = _eat_decision) - &_cards[curi][0] << endl;
				}
				else { cin >> a; eat = &_cards[curi][a]; }
				_players[vmi]->_score += calcu_score(*eat);
				(&_cards[curi + 1][0] + (eat - &_cards[curi][0]))->clear();
				(&_cards[curi + 1][0] + (eat - &_cards[curi][0]))->push_back(vmin);
				_punish_infos.push_back(PunishInfo(*players()[vmi], *eat));
			}
			else if (cards(NEXT_ROUND)[ci].size() == 5)
			{
				_players[vmi]->_score += calcu_score(cards(NEXT_ROUND)[ci]);
				_cards[curi + 1][ci].clear();
				_cards[curi + 1][ci].push_back(vmin);
				_punish_infos.push_back(PunishInfo(*players()[vmi], _cards[curi][ci]));
			}
			else _cards[curi + 1][ci].push_back(vmin);
		}

		on_round_end(curi);
		if (!my_cards(NEXT_ROUND).size()) break;
		curi++;
	}
	on_game_end();
	delete[] inp;
}

const int Agent::trans_index(int round_index) const
{
	switch (round_index)
	{
	case CURRENT_ROUND:
		return curi;
	case LAST_ROUND:
		return curi - 1;
	case NEXT_ROUND:
		return curi + 1;
	default:
		return round_index;
	}
}

void Agent::init_score_table()
{
	if (Agent::_score_table[0]) return;
	for (int i = 0; i < CARD_NUM + 1; ++i)
	{
		Agent::_score_table[i] = 1;
		if (!(i % 5)) Agent::_score_table[i] = 2;
		if (!(i % 10)) Agent::_score_table[i] = 3;
		if (!(i % 11)) Agent::_score_table[i] = 5;
	}
	Agent::_score_table[55] = 7;
}
int Agent::get_score(Card card) const
{
	return Agent::_score_table[card];
}
int Agent::calcu_score(const vector<Card>& cards) const
{
	int re = 0;
	for (auto a : cards)
		re += get_score(a);
	return re;
}
void Agent::decide_eat(const Cluster& cluster)
{
	if (&cards()[0] > &cluster || &cards()[3] < &cluster)
		throw "ERR03 you make an error decision : you can only decide a cluster in current round";
	_eat_decision = const_cast<Cluster*>(&cluster);
}

class SimpleAgent : public Agent
{
public:
	virtual void on_my_turn(int round_index) override
	{
		decide_card(*(my_cards().begin()));
	}
	virtual void on_eat_cluster(int round_index) override
	{
		decide_eat(cards()[0]);
	}
};

#include "stdafx.h"
int _tmain(int argc, _TCHAR* argv[])
{
	SimpleAgent agent;
	agent.Run();
	return 0;
}