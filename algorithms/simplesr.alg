$label "Simple SR"
#require "ltline.ind"
#require "utline.ind"

UTLine utl(1.0);
LTLine ltl(1.0);

candle top = candles[index];

if (top.close < utl[index] && top.open > utl[index])
{
	sell();
}
else if (top.close > ltl[index] && top.open < ltl[index - 1])
{
	buy();
}
