const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <style>
  body {
    background-color: #45b6fe;
    font-family: Arial, sans-serif;
  }
  form {
    background-color: white;
    padding: 20px;
    border-radius: 10px;
    margin: 0 auto;
    width: 300px;
  }
  input[type=text], input[type=password] {
    width: 100%;
    padding: 12px 20px;
    margin: 8px 0;
    display: inline-block;
    border: 1px solid #ccc;
    border-radius: 4px;
    box-sizing: border-box;
  }
  input[type=submit] {
    width: 100%;
    background-color: #4CAF50;
    color: white;
    padding: 14px 20px;
    margin: 8px 0;
    border: none;
    border-radius: 4px;
    cursor: pointer;
  }
  input[type=submit]:hover {
    background-color: #45a049;
  }
  </style>
</head>
<body>
  <form action="/connect" method="post">
    <label for="ssid">SSID:</label>
    <input type="text" id="ssid" name="ssid"><br><br>
    <label for="password">Password:</label>
    <input type="password" id="password" name="password"><br><br>
    <input type="submit" value="Submit">
  </form> 
</body>
</html>
)rawliteral";
