'use strict';

var assets = [];
var algorithms = [];
var asset_labels = [];
var current_account = -1;


var algoSelect = $("#algo-select");
var tickerSelect = $("#ticker-select");
var typeSelect = $("#type-select");

var watchButton = $("#watch-button");
var backtestButton = $("#backtest-button");
var shutdownButton = $("#shutdown-button");

var contentWindow = $("#content");
var accinfoField = $("#accinfo-field");

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
			asset_labels = data.types;
			
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

			get_accinfo();
			toggle_watch();
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

		get_accinfo();
		toggle_watch();
	});

	tickerSelect.change(() => 
	{
		get_accinfo();
		toggle_watch();
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


	var watching = -1;
	var watching_interval;

	function get_accinfo()
	{
		if (current_account == assets[tickerSelect.val()].type)
		{
			return;
		}
		current_account = assets[tickerSelect.val()].type;
		$.get(
		{
			url: "/accinfo",
			dataType: "json",
			data: {
				"asset_type": current_account,
			},
			success: function (data, status)
			{
				accinfoField.empty();
				accinfoField.append(
				`
					<div class="row border bg-light text-dark p-3 font-weight-bold">
						<div class="col">${asset_labels[current_account]} Account:</div>
						<div class="col">Balance: $${data.balance.toFixed(2)}</div>
						<div class="col">Buying Power: $${data.buying_power.toFixed(2)}</div>
						<div class="col">Equity: $${data.equity.toFixed(2)}</div>
					</div>
				`);
			}
		});
	}

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

				contentWindow.empty();
				contentWindow.append(
				`
					<div id="lwindow" class="col-2 bg-light p-5"></div>
					<div id="rwindow" class="col-10"></div>
				`);
				

				// handling acc data

				$("#lwindow").append(`
					Live: ${data.asset.live}
					<hr>
					Paper: ${data.asset.paper}
					<hr>
					Shares: ${data.asset.shares}
					<hr>
					Risk: ${data.asset.risk}
					<hr>
				`);

				// handling plot data
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

				Plotly.newPlot("rwindow", chartdata, layout);

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

					contentWindow.empty();
					//contentWindow.append(`<div class="row bg-light p-4" id="res-row"></div>`);
					for (let i = 0; i < data.length; i++)
					{

						let ranges_str = "";
						if (data[i].ranges == undefined)
						{
							console.error(`Failed to get result[${i}] from server`);
							continue;
						}
						for (let j = 0; j < data[i].ranges.length; j++)
						{
							if (j > 0) ranges_str += ", ";
							ranges_str += data[i].ranges[j].toString();
						}

						contentWindow.append(
						`
							<div class="col border m-1 p-3 bg-light">
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

	function toggle_watch()
	{
		if (watching == tickerSelect.val())
		{
			return;
		}
		clearInterval(watching_interval);
		console.log(`Stopped watching ${watching}!`);
		// Stopping observation

		get_watch();
		watching_interval = setInterval(get_watch, 60000);
		watching = tickerSelect.val();
		console.log(`Now watching ${watching}...`);
	}

	backtestButton.click(() =>
	{
		if (watching)
		{
			clearInterval(watching_interval);
			watching = false;
			console.log("Stopped watching...");
		}
		console.log("Backtesting...");
		get_backtest();
	});
});