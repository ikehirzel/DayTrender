'use strict';

var assets = [];
var algorithms = [];

var algoSelect = $("#algo-select");
var tickerSelect = $("#ticker-select");
var typeSelect = $("#type-select");

var watchButton = $("#watch-button");
var backtestButton = $("#backtest-button");
var shutdownButton = $("#shutdown-button");

var chartWindow = $("#chart-window");

$("document").ready(() => 
{
	$.get(
	{
		url: "/data",
		success: function (data, status) 
		{
			console.log("Data:", data);
			
			// default setting is to show all in a large pool
			// the user can then organize by type if desired
			assets = data.assets;
			
			typeSelect.append(`<option value=-1>All</option>`);
			for (let i = 0; i < data.types.length; i++)
			{
				typeSelect.append(`<option value=${i}>${data.types[i]}</option>`);
			}

			for (let i = 0; i < assets.length; i++) 
			{
				tickerSelect.append(`<option value=${i}>${assets[i].ticker}</option>`);
			}
			
			for (let i = 0; i < data.algorithms.length; i++)
			{
				algorithms[i] = data.algorithms[i];
				algoSelect.append(`<option value="${i}">${algorithms[i]}</option>`);
			}
		}
	});

	// callback for organizing assets
	typeSelect.change(() =>
	{
		let val = typeSelect.val();
		tickerSelect.empty();

		// if it is set to all, then put all of them
		if (val == -1)
		{
			for (let i = 0; i < assets.length; i++)
			{
				tickerSelect.append(`<option value=${i}>${assets[i].ticker}</option>`);
			}
		}
		// otherwise put all that match the type
		else
		{
			for (let i = 0; i < assets.length; i++)
			{
				if (assets[i].type == val)
				{
					tickerSelect.append(`<option value=${i}>${assets[i].ticker}</option>`);
				}
			}
		}
	});

	// callback for shutting down server
	shutdownButton.click(() => {
		$.get(
			{
				url: "/shutdown",
				success: function (data, status) {
					console.log(data);
					/*
					setTimeout(()=>{
						location.reload();
					}, 12000);
					*/
				}
			});
	});


	var watching = false;
	var watching_interval;

	function get_watch()
	{
		let index = tickerSelect.val();
		console.log("Asset index:", index);

		$.get({
			url: "/watch",
			dataType: "json",
			data: {
				"index": index,
			},
			success: function (data, status)
			{
	
				console.log("Watch data:", data);
				chartWindow.empty();
				var i_traces = [];

				var maxVolume = data.volume[0];
				for (let i = 0; i < data.low.length; i++) {
					maxVolume = (data.volume[i] > maxVolume) ? data.volume[i] : maxVolume;
				}

				var priceTrace = {
					type: "candlestick",
					open: data.open,
					high: data.high,
					low: data.low,
					close: data.close,
					x: data.x,
					yaxis: "y2",
					name: "OHLC",
					increasing: { line: { color: '#00CC00' } },
					decreasing: { line: { color: '#CC0000' } }
				};

				var volumeTrace = {
					x: data.x,
					y: data.volume,
					type: "bar",
					yaxis: "y",
					name: "Volume"
				};


				for (let i = 0; i < data.indicators.length; i++) {
					let ind = data.indicators[i];
					i_traces[i] = {
						x: data.x,
						y: ind.data,
						type: "scatter",
						yaxis: "y2",
						name: ind.label
					};
				}

				var chartdata = [priceTrace, volumeTrace];

				for (let i = 0; i < i_traces.length; i++) {
					chartdata.push(i_traces[i]);
				}

				let title = data.ticker + " @ " + data.interval + "sec";

				var layout = {
					title: title,
					xaxis:
					{
						title: "Time",
						type: "linear"
					},
					yaxis:
					{
						title: "Volume",
						type: "linear",
						range: [0, maxVolume * 2],
						side: "right"
					},
					yaxis2:
					{
						title: "Price",
						overlaying: "y1",
						type: "linear"
					}
				};

				Plotly.newPlot("chart-window", chartdata, layout);

			}
		});
	}

	function get_backtest()
	{
		var asset_index = tickerSelect.val();
		var algo_index = algoSelect.val();

		console.log("Asset:", asset_index);
		console.log("Algorithm:", algo_index);

		$.get(
			{
				url: "/backtest",
				dataType: "json",
				data: {
					"asset": asset_index,
					"algorithm": algo_index
				},
				success: function (data, status)
				{
					console.log("Response:", data);
					chartWindow.empty();
					chartWindow.append(`<div class="row bg-light p-4" id="res-row"></div>`);
					for (let i = 0; i < data.length; i++)
					{

						let ranges_str = "";
						for (let j = 0; j < data[i].ranges.length; j++)
						{
							if (j > 0) ranges_str += ", ";
							ranges_str += data[i].ranges[j].toString();
						}

						$("#res-row").append(
						`
							<div class="col border m-1 p-3">
								Buys        :    ${data[i].buys}<br>
								Sells       :    ${data[i].sells}<br>
								Interval    :    ${data[i].interval}<br>
								Ranges      :    ${ranges_str}<br>
								Elapsed Hrs :    ${data[i].elapsedhrs.toFixed(4)} (${(data[i].elapsedhrs / 24).toFixed(4)} days)<br>
								<hr>
								Initial     :  $ ${data[i].initial}<br>
								Shares      :    ${data[i].shares.toFixed(4)}<br>
								Balance     :  $ ${data[i].balance.toFixed(4)}<br>
								Equity      :  $ ${data[i].equity.toFixed(4)}<br>
								<hr>
								Net Return  :  $ ${data[i].netreturn.toFixed(4)}<br>
								% Return    :  % ${data[i].preturn.toFixed(4)}<br>
								<hr>
								Hr Return   :  $ ${data[i].hrreturn.toFixed(4)}<br>
								Hr% Return  :  % ${data[i].phrreturn.toFixed(4)}<br>
								<hr>
								Win Rate    :  % ${data[i].winrate.toFixed(4)}<br>
								B Win Rate  :  % ${data[i].bwinrate.toFixed(4)}<br>
								S Win Rate  :  % ${data[i].swinrate.toFixed(4)}<br>
							</div>
						`);
					}
	
				}
			});
	}

	watchButton.click(() =>
	{
		if (watching)
		{
			clearInterval(watching_interval);
			watching = false;
			console.log("Stopped watching...");
		}
		else
		{
			get_watch();
			watching_interval = setInterval(get_watch, 60000);
			watching = true;
			console.log("Now watching...");
		}
	});

	backtestButton.click(() =>
	{
		console.log("backtesting...");
		get_backtest();
	});
});