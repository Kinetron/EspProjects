//First init page if device is run.
const char defaultPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1.0" />
	<title>Device settings</title>
	<style>
		input[type=submit],
		input[type=button],
		button {
			height: 35px;
			font-size: 20px;
			width: 90%;
			max-width: 300px;
			margin: 8px 5px;
			background-color: #4CAF50;
			border: none;
			border-radius: 8px;
			color: white;
			cursor: pointer;
			padding: 0px 10px;
		}
				
		body {
			font-family: Verdana;
			margin-top: 15px;
		}

		label {
			white-space: nowrap;
			font-size: 20px;
			margin: 0 5px;
		}
		p {
			font-size: 20px;
			border-radius: 10px;
			background-color: #f2f2f2;
			box-shadow: #aaa 0px 0px 10px;
			margin: 15px;
		}

		.access-token-input 
		{
			width: 400px;
		}

		.wifi-stations-block {
			font-size: 20px;
			border-radius: 10px;
			background-color: #ffffff;
			box-shadow: #fafad2 0px 0px 10px;
			margin: 15px;
		}		
	</style>
</head>
<body>
<p>
  Wi-Fi settings
</p>

Available stations:<br>
		<div class="wifi-stations-block">
	      @stations
		</div>
 <form method='get' action='scanWifi'>
   <input type='submit' value="scanWifi">
 </form>
<p>
	<form method='get' action='wifiSet'>
		<table>
			<tr>
				<td>
					<label>SSID </label>
				</td>
				<td>
					<input name='ssid' length=32>
				</td>
			</tr>
			<tr>
				<td>
					<label>Password </label>
				</td>
				<td>
						<input name='pass' length=64>
				</td>
			</tr>
		</table>
			<input type='submit' value="ave wifi">
	</form>
</p>
<p>
	Telegram bot settings<br>
</p>
<p>
	<div>
		Warning: Keep your token secure and store it safely, it can be used by anyone to control your bot.
	</div>
	<br>
	<form method='get' action='bot_set'>
			<table>
			<tr>
				<td>
					<label>Access token</label>
				</td>
					<td>
						<input name='token' length=640 class="access-token-input">
				</td>
			</tr>
			<tr>
				<td>
					<label>Сlient id </label>
				</td>
				<td>
					<input name='client_id' length=64>
				</td>
			</tr>
		</table>
		<input type='submit' value="Save bot">
	</form>
</p>
</body>
</html>
)=====";