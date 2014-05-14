# Usage:
# please implent on_my_turn and on_eat_cluster and send to 
# an Agent instance
# on_my_turn:
# 	you must make decision which card to use after on_my_turn
# 	through calling decide_card. It doesn't matter to call 
# 	decide_card ealy as long as this turn started.
# on_eat_cluster:
# 	you must make decision which cluster to eat after 
# 	on_eat_cluster through calling decide_eat. It doesn't 
# 	matter to call decide_eat ealy as long as this turn 
# 	started.
require 'set'
class Integer
	N_BYTES = [42].pack('i').size
	N_BITS = N_BYTES * 8
	MAX = 2 ** (N_BITS - 2) - 1
	MIN = -MAX - 1
end
class AgentRef
	def initialize (name, index)
		@name = name
		@index = index
		@score = 0
	end
	attr_reader :name, :index
	attr_accessor :score
end

Cluster = Array

class AgentDic < Array
	def [] (i)
		 return super.[](i.index) if (AgentRef === i)
		 super.[](i)
	end
end

class PunishInfo
	def initialize(who, which)
		@who = who
		@which = which
	end
	attr_reader :who, :which
end

class Agent
	CARD_NUM = 104
	def initialize
		@clusters = []
		@on_game_start = @on_round_start = @on_round_end = @on_game_end = Proc.new {}
	end
	
	def self.event(*names)
		names.each{ |name|
			define_method name do |&prc|
				if prc then instance_variable_set ('@'+name.to_s).to_sym, prc
				else instance_variable_get ('@'+name.to_s).to_sym end
			end
		}
	end
	event :on_game_start, :on_round_start, :on_eat_cluster
	event :on_round_end, :on_game_end, :on_my_turn
	attr_reader :my_index, :players, :curi, :droped_cards
	attr_reader :who_use
	alias :current_index :curi
	def me
		@players[my_index]
	end
	def clusters(round = -1)
		@clusters[transi round]
	end
	alias :cards :clusters
	def mycards(round = -1)
		@mycards[transi round]
	end
	def used_cards(round = -1)
		@used_cards[transi round]
	end
	def punish_info(round = -1)
		@punish_info[transi round]
	end

	def decide_card(card)
		raise 'Please! It\'s not your card!' unless mycards.include? card
		@decision_card = card
	end
	def decide_eat(cluster)
		raise 'Please! It\'s not a valid cluster in current' unless clusters.include? cluster
		@decision_eat = cluster
	end

	def self.init_score_table
		@@score_table = Array.new CARD_NUM, 1
		CARD_NUM.times{ |i|
			@@score_table[i] = 2 if (i%5) == 0
			@@score_table[i] = 3 if (i%10) == 0
			@@score_table[i] = 5 if (i%11) == 0
	   	}
		@@score_table[55] = 7
	end
	init_score_table
	def calcu_score (card)
		case card
		when Fixnum then @@score_table[card]
		when Array then card.inject(0) {|re,x| re + @@score_table[x]}
		else raise 'Bad Argument!'
		end
	end

	private
	def clear_decision
		@decision_card = nil
		@decision_eat = nil
	end
	def transi(ri) #translate (round_index)
		case ri
		when -1 then @curi
		when -2 then @curi-1
		when -3 then @curi+1
		else ri
		end
	end
	def init
		@used_cards = []
		@punish_info = []
		@who_use = Array.new CARD_NUM
		@curi = 0
		
		n, m, p = gets.split(' ').map{|x| x.to_i}
		@players = Array.new n+1
		n.times {|i| @players[i] = AgentRef.new gets.chomp, i }
		@players[n] = AgentRef.new 'me', @my_index = n
		@mycards = [gets.split(' ').map{|x| x.to_i}]
		@droped_cards = gets.split(' ').map{|x| x.to_i}
		@clusters = [gets.split(' ').map{|x| [x.to_i]}]
	end
	public
	def run
		init
		@on_game_start && @on_game_start.call
		while @mycards[@curi].count != 0 do
			clear_decision
			@on_round_start && @on_round_start.call
			@on_my_turn && @on_my_turn.call
			raise 'You forgot to make dicision = =' unless @decision_card
			puts @decision_card

			@punish_info.push nil
			@mycards.push @mycards.last.clone
			@mycards.last.delete @decision_card
			inp= gets.split(' ').map{|x| x.to_i}.push @decision_card
			@used_cards.push AgentDic.new inp
			@players.count.times{|i| @who_use[inp[i]]=@players[i]}
			@clusters.push @clusters[@curi].map{|x| x.clone}
			@players.each { |er|
				vmin, who = *inp.each_with_index.min
				inp[who] = Integer::MAX
				cv, ci = *@clusters[@curi+1].each_with_index.select{|x, i| x.last < vmin }.max
				if !cv then
					if who == @my_index then
						@on_eat_cluster && @on_eat_cluster.call
						raise 'You forgot make decision !' unless @decision_eat
						puts (eati = clusters.index(@decision_eat))
					else eati = gets.to_i
					end
					@players[who].score += calcu_score @clusters[@curi][eati]
					@clusters[@curi+1][eati].clear.push vmin
					@punish_info[-1] =  PunishInfo.new(@players[who], @clusters[@curi][eati])
				elsif @clusters[@curi+1][ci].count == 5 then
					@players[who].score += calcu_score @clusters[@curi+1][ci]
					@clusters[@curi+1][ci].clear.push vmin
				else @clusters[@curi+1][ci].push vmin
				end
			}
			@on_round_end && @on_round_end.call
			break if @mycards[@curi+1].count == 0
			@curi+=1
		end
	end
end

agt = Agent.new
agt.on_eat_cluster { agt.decide_eat(agt.clusters[0]) }
agt.on_my_turn { agt.decide_card agt.mycards.first }
agt.run
