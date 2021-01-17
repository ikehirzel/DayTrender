$label "Simple SR"

#require "ltline.ind"
#require "utline.ind"
#require "atline.ind"
#require "midline.ind"

UTLine utl(1.0);
ATLine atl(1.0);
LTLine ltl(1.0);
MidLine mid(1.0);

candle curr = candles.back();

bool upper_breach = false;
bool lower_breach = false;

for (unsigned i = length - 5; i < length - 1; i++)
{
	if (candles[i].close >= utl[i]) upper_breach = true;
	if (candles[i].close <= ltl[i]) lower_breach = true;
}

if (upper_breach && curr.close <= utl.back())
{
	sell();
}

if (lower_breach && curr.close >= ltl.back())
{
	buy();
}
