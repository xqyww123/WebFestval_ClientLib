require 'irb'
IRB.start(__FILE__)

class A
	@x = 1
	@@num = 0
	@@x = 10
	def initialize (x)
		@x = x
		@@num += 1
		@@x += 1
	end
	def to_s
		@x.to_s
	end
	def self.cnt
		@@num
	end
	attr_reader :x, :num
   	attr_accessor :ax

	def current
		@current
	end
	alias :@current :@curi
	def scuri(x)
		curi = x
	end
end

a = A.new 1
puts a.x
puts A.cnt
p a.ax
a.ax = 100
puts a.ax
a.scuri 4
p a.current
