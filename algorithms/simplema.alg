$label "Simple MA"

$require "indicators/ema.ind" as EMA

EMA longma(1.0);
EMA shortma(0.25);

double lastShort = shortma[index - 1];
double lastLong = longma[index - 1];

//bullish crossover
if(shortma[index] > longma[index] && lastShort < lastLong)
{
	return ACTION_BUY;
}
//bearish crossover
else if (shortma[index] < longma[index] && lastShort > lastLong)
{
	return ACTION_SELL;
}

return ACTION_NOTHING;

