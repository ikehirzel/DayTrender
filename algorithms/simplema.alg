$label "Simple MA"

#require "ema.ind"

EMA longma(1.0);
EMA shortma(0.25);

double lastShort = shortma[index - 1];
double lastLong = longma[index - 1];

//bullish crossover
if(shortma[index] > longma[index] && lastShort < lastLong)
{
	buy();
}

//bearish crossover
else if (shortma[index] < longma[index] && lastShort > lastLong)
{
	sell();
}
