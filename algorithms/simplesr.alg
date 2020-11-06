$label "Simple SR"
#require "ltline.ind"
#require "utline.ind"

UTLine utl(1.0);
LTLine ltl(1.0);

candle top = candles.back();

if (top.close >= utl.back() || top.open >= utl.back())
{
	sell();
}
else if (top.close <= ltl.back() && top.open <= ltl.back())
{
	buy();
}
