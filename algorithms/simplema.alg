$label "Simple MA"

#require "ema.ind"

EMA longma(1.0);
EMA shortma(0.25);

double lastShort = shortma[length - 2];
double lastLong = longma[length - 2];

//bullish crossover
if(shortma.back() > longma.back() && lastShort < lastLong)
{
	buy();
}
//bearish crossover
else if (shortma.back() < longma.back() && lastShort > lastLong)
{
	sell();
}
