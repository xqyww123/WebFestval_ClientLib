using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Card = System.Int32;

namespace Cslib
{
    public static class Factory
    {
        public static IEnumerator<T> To<T>(this IEnumerable<T> arr, out T to)
        {
            var re = arr.GetEnumerator();
            if (!re.MoveNext()) re.Dispose();
            to = re.Current;
            return re;
        }
        public static IEnumerator<T> To<T>(this IEnumerator<T> arr, out T to)
        {
            var re = arr;
            if (!re.MoveNext()) re.Dispose();
            to = re.Current;
            return re;
        }

        public static int[] ReadInts()
        {
            return Console.ReadLine().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries).Select(int.Parse).ToArray();
        }

        public static int IndexOf<T>(this IEnumerable<T> arr, T val)
        {
            int re = 0;
            foreach (var a in arr)
            {
                if (a.Equals(val))
                    return re;
                re++;
            }
            return re;
        }
    }

    public class AgentRef : IComparable<AgentRef>
    {
        public AgentRef(string name, int index)
        {
            Name = name;
            Index = index;
        }
        public string Name;
        public int Index;
        public int Score;
        public int CompareTo(AgentRef other)
        {
            return Index - other.Index;
        }
    }

    public class AgentDic<T> : List<T>
    {
        public AgentDic(int num) : base(num) { for(int i=0;i<num;++i) Add(default(T)); }
        public T this[AgentRef agent] { get { return base[agent.Index]; } set { base[agent.Index] = value; } }
    }
    public class Cluster : List<Card>
    {
        public Cluster() { }
        public Cluster(IEnumerable<Card> cards) : base(cards) { }
    }
    public struct PunishInfo
    {
        public AgentRef Who;
        public Cluster Which;
    }

    public abstract class Agent
    {
        private int decision_card;
        private Cluster decision_eat;
        private List<Cluster[]> clusters;
        private List<HashSet<Card>> mycards;
        private List<AgentDic<Card>> used_cards;
        private List<PunishInfo> eat_info;

        private int Translate(int round_index)
        {
            switch (round_index)
            {
                case -1:
                    return CurrentIndex;
                case -2:
                    return CurrentIndex - 1;
                case -3:
                    return CurrentIndex + 1;
                default:
                    return round_index;
            }
        }
        static Agent()
        {
            init_score_table();
        }

        public abstract string GetMyId();

        public const int CardNum = 104;
        public int MyIndex { get; private set; }
        public AgentRef Me { get { return Players[MyIndex]; } }
        public AgentRef[] Players { get; private set; }
        public int CurrentIndex { get; private set; }
        public Cluster[] Clusters(int round_index = -1)
        {
            return clusters[Translate(round_index)];
        }
        public HashSet<Card> MyCards(int round_index = -1)
        {
            return mycards[Translate(round_index)];
        }
        public AgentDic<Card> UsedCards(int round_index = -1)
        {
            return used_cards[Translate(round_index)];
        }
        public PunishInfo PunishInfo(int round_index = -1)
        {
            return eat_info[Translate(round_index)];
        }

        public Card[] DropedCards { get; private set; }
        public AgentRef[] WhoUse { get; private set; }

        public void DecideCard(Card card)
        {
#if DEBUG
            if (!mycards[CurrentIndex].Contains(card))
                throw new ArgumentException("Card not exists in your cards");
#endif
            decision_card = card;
        }
        public void DecideEate(Cluster cluster)
        {
#if DEBUG
            if (!Clusters().Contains(cluster))
                throw new ArgumentException("Cluster not exists in current clusters");
#endif
            decision_eat = cluster;
        }

        public virtual void OnGameStart() { }
        public virtual void OnRoundStart() { }
        public abstract void OnMyTurn();
        public abstract void OnEatCluster();
        public virtual void OnRoundEnd() { }
        public virtual void OnGameEnd() { }

        private void ClearLastDecision()
        {
            decision_card = -1;
            decision_eat = null;
        }
        private void Init()
        {
            used_cards = new List<AgentDic<int>>() { new AgentDic<int>(0) };
            eat_info = new List<PunishInfo>();
            WhoUse = new AgentRef[CardNum];
            CurrentIndex = 0;

            int n, m, p; 
            Factory.ReadInts().To(out n).To(out m).To(out p);
            Players = new AgentRef[n + 1];
            for (int i = 0; i < n; i++)
                Players[i] = new AgentRef(Console.ReadLine(), i);
            MyIndex = n;
            Players[n] = new AgentRef("me", n);
            mycards = new List<HashSet<int>>() { new HashSet<int>(Factory.ReadInts()) };
            DropedCards = Factory.ReadInts();
            clusters = new List<Cluster[]>() { Factory.ReadInts().Select(a => new Cluster(new int[]{a})).ToArray() };
        }
        public void Run()
        {
            Init();
            OnGameStart();
            var inp = new int[Players.Length];
            while (true)
            {
                ClearLastDecision();
                OnRoundStart();
                OnMyTurn();
                if (decision_card == -1) throw new Exception("You forget make decision");
                Console.WriteLine(decision_card);
                used_cards.Add(new AgentDic<int>(Players.Length));
                used_cards[CurrentIndex + 1][MyIndex] = decision_card;
                WhoUse[decision_card] = Me;
                mycards.Add(new HashSet<int>(mycards[CurrentIndex]));
                mycards[CurrentIndex + 1].Remove(decision_card);
                /* ==== After Decision ==== */
                Factory.ReadInts().CopyTo(inp, 0);
                for (int i = 0; i < Players.Length-1; i++)
                {
                    used_cards[CurrentIndex + 1][i] = inp[i];
                    WhoUse[inp[i]] = Players[i];
                }
                inp[MyIndex] = decision_card;
                clusters.Add(clusters[CurrentIndex].Select(a => new Cluster(a)).ToArray());
                foreach (var er in Players)
                {
                    int min = int.MaxValue, who = -1;
                    for (int i=0;i<Players.Length;++i)
                        if (min > inp[i])
                        {
                            min = inp[i];
                            who = i;
                        }
                    inp[who] = int.MaxValue;
                    int ci = -1, cv = -1;
                    for (int j = 0; j < 4; ++j)
                        if (clusters[CurrentIndex + 1][j][clusters[CurrentIndex + 1][j].Count - 1] < min &&
                            clusters[CurrentIndex + 1][j][clusters[CurrentIndex + 1][j].Count - 1] > cv)
                        { cv = clusters[CurrentIndex + 1][j][clusters[CurrentIndex + 1][j].Count - 1]; ci = j; }
                    if (cv == -1)
                    {
                        int eat_index;
                        if (who == MyIndex)
                        {
                            OnEatCluster();
                            if (decision_eat == null)
                                throw new Exception("You forget make eat decision!");
                            Console.WriteLine(eat_index = clusters[CurrentIndex].IndexOf(decision_eat));
                        }
                        else eat_index = int.Parse(Console.ReadLine());
                        Players[who].Score += CalcuScore(clusters[CurrentIndex][eat_index]);
                        clusters[CurrentIndex + 1][eat_index].Clear();
                        clusters[CurrentIndex + 1][eat_index].Add(min);
                        eat_info.Add(new PunishInfo(){ Which = clusters[CurrentIndex][eat_index], Who = Players[who]});
                    }
                    else if (clusters[CurrentIndex + 1][ci].Count == 5)
                    {
                        Players[who].Score += CalcuScore(clusters[CurrentIndex + 1][ci]);
                        clusters[CurrentIndex+1][ci].Clear();
                        clusters[CurrentIndex + 1][ci].Add(min);
                    }
                    else clusters[CurrentIndex + 1][ci].Add(min);
                }
                OnRoundEnd();
                if (mycards[CurrentIndex+1].Count == 0) break;
                CurrentIndex++;
            }
            OnGameEnd();
        }

        static readonly int[] _score_table = new int[CardNum+1];
        private static void init_score_table()
        {
            if (_score_table[0] != 0) return;
            for (int i = 0; i < CardNum + 1; ++i)
            {
	            _score_table[i] = 1;
	            if ((i % 5) == 0) _score_table[i] = 2;
	            if ((i % 10) == 0) _score_table[i] = 3;
	            if ((i % 11) == 0) _score_table[i] = 5;
            }
            _score_table[55] = 7;
        }
        public int CalcuScore(Card card)
        {
            return _score_table[card];
        }
        public int CalcuScore(IEnumerable<Card> cards)
        {
            return cards.Sum(a => CalcuScore(a));
        }
    }

    public class SimpleAgent : Agent
    {
        public override string GetMyId()
        {
            return "Alalal";
        }

        public override void OnMyTurn()
        {
            DecideCard(MyCards().First());
        }

        public override void OnEatCluster()
        {
            DecideEate(Clusters()[0]);
        }
    }

    public class Program
    {
        public static void Main(string[] arg)
        {
            var agent = new SimpleAgent();
            agent.Run();
        }
    }
}
