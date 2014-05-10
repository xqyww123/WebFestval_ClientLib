import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Scanner;

class AgentRef
{
	public AgentRef(String name, int index)
	{
		Name = name;
		Index = index;
		score = 0;
	}
	public final String Name;
	public final int Index;
	public int score;
}
class Card
{
	public Card(int _number) { number = _number; }
	public final int number;
	public boolean equals(Object o)
	{
		if (o instanceof Card)
			return ((Card) o).number == number;
		return false;
	}
	@Override
	public int hashCode()
	{
		return number;
	}
}
class Cluster extends ArrayList<Card> { }
class AgentDic<T> extends ArrayList<T>
{
	public AgentDic(int num) { super(num); }
	public T get(AgentRef agt) { return super.get(agt.Index); }
	public void set(AgentRef agt, T val) { super.set(agt.Index, val);}
}
class punishInfo
{
	public punishInfo(AgentRef who, Cluster which) { Who = who; Which = which; }
	public final AgentRef Who;
	public final Cluster Which; 
}

abstract class Agent
{
	public static final int CARDNUM = 104;
	public Agent() { init_score_table(); }
	
	private int _myindex;
	private AgentRef[] _players;
	private int curi;
	private ArrayList<Cluster[]> _clusters;
	private ArrayList<HashSet<Card>> _mycards;
	private ArrayList<punishInfo> _punish_info;
	private ArrayList<AgentDic<Card>> _used_cards; 
	private Card[] _droped_cards;
	private AgentRef[] _who_use;
	private Card _decision_card;
	private Cluster _decision_eat;
	
	private int translate(int round_index)
    {
        switch (round_index)
        {
            case -1:
                return curi();
            case -2:
                return curi() - 1;
            case -3:
                return curi() + 1;
            default:
                return round_index;
        }
    }
	
	public abstract String getMyId();
	public int myIndex() { return _myindex; }
	public AgentRef me() { return _players[myIndex()]; }
	public AgentRef[] players() { return _players; } 
	public int curi() { return curi; }
	public Cluster[] clusters() { return clusters(-1); }
	public Cluster[] clusters(int round_index) { return _clusters.get(translate(round_index)); }
	public HashSet<Card> myCards() { return myCards(-1); }
    public HashSet<Card> myCards(int round_index) { return _mycards.get(translate(round_index)); }
    public AgentDic<Card> usedCards() { return usedCards(-1); }
    public AgentDic<Card> usedCards(int round_index) { return _used_cards.get(translate(round_index)); }
    public punishInfo punishInfo() { return punishInfo(-1); }
    public punishInfo punishInfo(int round_index) { return _punish_info.get(translate(round_index)); }
	
    public Card[] dropedCards() { return _droped_cards; }
    public AgentRef[] whoUse() { return _who_use; }
    
    public void decideCard(Card card) throws Exception
    {
        if (!_mycards.get(curi()).contains(card))
            throw new Exception("Card not exists in your cards");
        _decision_card = card;
    }
    public void decideEate(Cluster cluster) throws Exception
    {
    	int a;
    	for (a = 0; a < 4 && !_clusters.get(curi)[a].equals(_decision_eat); ++a)
        if (a == 4)
            throw new Exception("Cluster not exists in current _clusters");
        _decision_eat = cluster;
    }
    
    public void onGameStart() { }
    public void onRoundStart() { }
    public abstract void onMyTurn() throws Exception;
    public abstract void onEatCluster() throws Exception;
    public void onRoundEnd() { }
    public void onGameEnd() { }
    
    private void clearLastDecision()
    {
        _decision_card = new Card(-1);
        _decision_eat = null;
    }
    private void init() throws IOException
    {
        _used_cards = new ArrayList<AgentDic<Card>>();
        _used_cards.add(new AgentDic<Card>(0));
        _punish_info = new ArrayList<punishInfo>();
        _who_use = new AgentRef[CARDNUM];
        curi = 0;

        Scanner scan = new Scanner(System.in);
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        int n = scan.nextInt(), m = scan.nextInt(), p = scan.nextInt();
        _players = new AgentRef[n + 1];
        for (int i = 0; i < n; i++)
            _players[i] = new AgentRef(in.readLine(), i);
        _myindex = n;
        _players[n] = new AgentRef("me", n);
        _mycards = new ArrayList<HashSet<Card>>();
        _mycards.add(new HashSet<Card>());
        for (int i=0;i<m;++i) _mycards.get(0).add(new Card(scan.nextInt()));
        _droped_cards = new Card[p];
        for (int i=0;i<p;++i) _droped_cards[i] = new Card(scan.nextInt());
        _clusters = new ArrayList<Cluster[]>();
        _clusters.add(new Cluster[4]);
        for (int i=0;i<4;++i) _clusters.get(0)[i] = new Cluster();
        for (int i=0;i<4;++i) _clusters.get(0)[i].add(new Card(scan.nextInt()));
    }
    public void Run() throws Exception
    {
        init();
        onGameStart();
        int[] inp = new int[_players.length];
        Scanner scan = new Scanner(System.in);
        while (true)
        {
            clearLastDecision();
            onRoundStart();
            onMyTurn();
            if (_decision_card.number == -1) throw new Exception("You forget make decision");
            System.out.println(_decision_card.number);
            _used_cards.add(new AgentDic<Card>(_players.length));
            _who_use[_decision_card.number] = me();
            _mycards.add(new HashSet<Card>(_mycards.get(curi)));
            _mycards.get(curi + 1).remove(_decision_card);
            /* ==== After Decision ==== */
            for (int i=0;i<_players.length -1 ;++i) inp[i] = scan.nextInt(); 
            for (int i = 0; i < _players.length-1; i++)
            {
                _used_cards.get(curi + 1).add(new Card(inp[i]));
                _who_use[inp[i]] = _players[i];
            }
            inp[_myindex] = _decision_card.number;
            _used_cards.get(curi + 1).add(_decision_card);
            _clusters.add(new Cluster[4]);
            for (int i=0;i<4;++i) _clusters.get(curi+1)[i] = (Cluster)_clusters.get(curi)[i].clone(); 
            for (AgentRef er : _players)
            {
                int min = Integer.MAX_VALUE, who = -1;
                for (int i=0;i<_players.length;++i)
                    if (min > inp[i])
                    {
                        min = inp[i];
                        who = i;
                    }
                inp[who] = Integer.MAX_VALUE;
                int ci = -1, cv = -1;
                for (int j = 0; j < 4; ++j)
                    if (_clusters.get(curi + 1)[j].get(_clusters.get(curi + 1)[j].size() - 1).number < min &&
                    		_clusters.get(curi + 1)[j].get(_clusters.get(curi + 1)[j].size() - 1).number > cv)
                    { cv = _clusters.get(curi + 1)[j].get(_clusters.get(curi + 1)[j].size() - 1).number; ci = j; }
                if (cv == -1)
                {
                    int eat_index;
                    if (who == _myindex)
                    {
                        onEatCluster();
                        if (_decision_eat == null)
                            throw new Exception("You forget make eat decision!");
                        for (eat_index = 0; eat_index < 4 && !_clusters.get(curi)[eat_index].equals(_decision_eat); ++eat_index)
                        System.out.println(eat_index);
                    }
                    else eat_index = scan.nextInt();
                    _players[who].score += calcuScore(_clusters.get(curi)[eat_index]);
                    _clusters.get(curi + 1)[eat_index].clear();
                    _clusters.get(curi + 1)[eat_index].add(new Card(min));
                    _punish_info.add(new punishInfo(_players[who], _clusters.get(curi)[eat_index]));
                }
                else if (_clusters.get(curi + 1)[ci].size() == 5)
                {
                    _players[who].score += calcuScore(_clusters.get(curi + 1)[ci]);
                    _clusters.get(curi+1)[ci].clear();
                    _clusters.get(curi + 1)[ci].add(new Card(min));
                }
                else _clusters.get(curi + 1)[ci].add(new Card(min));
            }
            onRoundEnd();
            if (_mycards.get(curi+1).size() == 0) break;
            curi++;
        }
        onGameEnd();
    }
    
    static int[] _score_table = new int[CARDNUM+1];
    private static void init_score_table()
    {	
        if (_score_table[0] != 0) return;
        for (int i = 0; i < CARDNUM + 1; ++i)
        {
            _score_table[i] = 1;
            if ((i % 5) == 0) _score_table[i] = 2;
            if ((i % 10) == 0) _score_table[i] = 3;
            if ((i % 11) == 0) _score_table[i] = 5;
        }
        _score_table[55] = 7;
    }
    public int calcuScore(Card card)
    {
        return _score_table[card.number];
    }
    public int calcuScore(List<Card> cards)
    {
    	int re = 0;
    	for (Card a : cards) re += calcuScore(a);
        return re;
    }
    
    public static void main(String[] args) throws Exception
    {
    	Agent agt = new SimpleAgent();
    	agt.Run();
    }
}

class SimpleAgent extends Agent
{
	@Override 
    public String getMyId()
    {
        return "asdwer";
    }

	@Override
    public void onMyTurn() throws Exception
    {
        decideCard(myCards().iterator().next());
    }

	@Override
    public void onEatCluster() throws Exception
    {
        decideEate(clusters()[0]);
    }
}
