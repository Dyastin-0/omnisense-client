const char authPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Omnisense</title>
  <style>
    :root {
      --base-color: rgb(230, 230, 230);
      --complement: rgb(210, 210, 210);
      --secondary-color: rgb(220, 220, 220);
      --text-color: rgb(80, 80, 80);
      --shadow: rgba(0, 0, 0, 0.1);
      --accent: rgba(31, 117, 254, 0.5);
      --description: rgb(132, 136, 132);
      --red: rgb(136, 0, 0);
    }

    :root.dark {
      --base-color: rgb(35, 35, 35);
      --secondary-color: rgb(45, 45, 45);
      --text-color: rgb(240, 240, 240);
      --complement: rgb(25, 25, 25);
      --shadow: rgba(0, 0, 0, 0.1);
      --accent: rgba(31, 117, 254, 0.5);
      --description: rgb(132, 136, 132);
    }

    * {
      padding: 0;
      margin: 0;
      font-family: 'Poppins', 'sans serif';
      transition: top 0.3s ease, background-color 0.3s ease, color 0.3s ease, height 0.3s ease, transform 0.3s ease, opacity 0.3s ease;
    }

    html, body {
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background-color: var(--complement);
    }

    .App {
      width: calc(100% - 18px);
      max-width: 2048px;
      display: flex;
      justify-content: center;
      row-gap: 9px;
      column-gap: 9px;
      padding: 9px;
      flex-wrap: wrap;
    }

    .auth {
      width: 267px;
      height: fit-content;
      padding: 18px;
      gap: 9px;
      border-radius: 9px;
      background-color: var(--base-color);
      display: flex;
      justify-content: center;
      align-items: center;
      flex-direction: column;
    }

    .auth button {
      width: 100%;
      padding: 5px;
      border-radius: 5px;
      border: none;
      outline: none;
      color: var(--text-color);
      background-color: var(--complement);
    }

    p, h6, h5, h4, h2, h1 {
      text-align: center;
      color: var(--text-color);
      overflow: hidden;
      text-overflow: ellipsis;
    }

    h6.right, p.right {
      text-align: right;
    }

    h2, h3 {
      color: var(--text-color);
      width: calc(100% - 8px);
      border-radius: 9px;
      text-align: center;
      text-overflow: none;
    }

    p {
      font-size: 14px;
      text-overflow: ellipsis;
      overflow: hidden;
    }

    p.center {
      text-align: center;
      width: 100%;
    }

    p.small {
      font-size: 11px;
    }

    .description {
      color: var(--description);
    }

    a {
      font-size: 14px;
      color: var(--text-color);
      width: fit-content;
    }

    i {
      color: var(--text-color);
    }

    input {
      width: 100%;
      padding: 9px;
      border-radius: 5px;
      box-sizing: border-box;
      outline: none;
      border: none;
      color: var(--text-color);
      background-color: var(--secondary-color);
    }

    input.width-half-parent {
      width: 50%;
    }

    input.small {
      font-size: 11px;
      padding: 5px;
    }

    .row {
      display: flex;
      flex-direction: row;
      gap: 9px;
      justify-content: center;
      align-items: center;
    }

    .row.left {
      justify-content: left;
    }

    .row.no-gap {
      gap: 0;
    }

    .row.space-between {
      justify-content: space-between;
    }

    .column {
      display: flex;
      flex-direction: column;
      gap: 9px;
    }

    .column.flex {
      flex-basis: 33.33333%;
    }

    .red {
      color: var(--red);
    }

    .nav-button {
      padding: 9px;
      font-size: 14px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      column-gap: 9px;
      border: none;
      border-radius: 9px;
      color: var(--text-color);
      background-color: transparent;
    }

    .nav-button.fit {
      width: fit-content;
    }

    .nav-button.link {
      width: fit-content;
      background-color: transparent;
      text-decoration: underline;
    }

    .nav-button.link:hover {
      color: var(--accent);
      background-color: transparent;
    }

    .nav-button.center {
      justify-content: center;
    }

    .nav-button.bold {
      font-weight: bold;
    }

    .nav-button.fixed {
      position: fixed;
      bottom: 9px;
      right: 9px;
    }

    .nav-button:hover {
      cursor: pointer;
      background-color: var(--accent);
    }

    .nav-button.round {
      padding-top: 18px;
      padding-bottom: 18px;
      border-radius: 50%;
    }

    .nav-button.small {
      font-size: 11px;
    }

    .message {
      display: flex;
      justify-content: center;
      align-items: center;
      background-color: var(--base-color);
      border-radius: 18px;
      width: 300px;
      height: 300px;
      min-width: calc(100% - 18px);
      padding: 9px;
    }

    .instance-container {
      display: flex;
      flex-direction: column;
      gap: 9px;
      width: 100%;
      align-items: center;
    }

    .instance-button {
      width: 100%;
      padding: 9px;
      border-radius: 5px;
      border: none;
      background-color: var(--complement);
      color: var(--text-color);
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .instance-button:hover {
      background-color: var(--accent);
    }
  </style>
</head>
<body>
  <div class="App">
    <div class='auth' id="auth-panel">
      <h2>Omnisense</h2>
      <p>Sign in to your account</p>
      <input id="email" placeholder="Email" type="email" enterKeyHint='Enter' required></input>
      <input id="password" placeholder="Password" type="password" enterKeyHint='Enter' required></input>
      <button class="nav-button center" id="sign-in">Sign in</button>
    </div>
  </div>

  <script>
    const app = document.querySelector('.App');
    const signInButton = document.getElementById('sign-in');

    const socket = new WebSocket(`ws://${location.hostname}:81`);

    signInButton.addEventListener('click', () => {
      const email = document.getElementById('email').value;
      const password = document.getElementById('password').value;

      if (!email || !password) {
        alert("Please enter both email and password");
        return;
      }

      const authRequest = {
        auth_request: {
          email: email,
          password: password
        }
      };

      socket.send(JSON.stringify(authRequest));
    });

    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log(data);

      if (data.auth_result?.isAuthenticated) {
        removeNode('auth-panel');
        displayMessage('Sign in success, you can now use the app.');
      } else {
        data.auth_result && alert(data.auth_result.message);
      }

      if (data.auth?.status === 'authenticated') {
        removeNode('auth-panel');
        displayMessage('This ESP32 is already authenticated, you can use the app.'); 
      }

      if (data.instances) {
        removeNode('auth-panel');
        removeNode('instance-container');
        displayInstanceButtons(data.instances);
      }
    };

    const removeNode = (id) => {
      const node = document.getElementById(id);
      if (node) node.remove();
    };

    const displayMessage = (message) => {
      const success = document.createElement('h3');
      success.textContent = message;
      success.className = 'center';
      app.appendChild(success);
    };

    const displayInstanceButtons = (instances) => {
      const instanceContainer = document.createElement('div');
      instanceContainer.className = 'auth';
			instanceContainer.id = 'instance-container';

      instances.forEach((instance) => {
        const button = document.createElement('button');
        button.textContent = instance;
        button.className = 'instance-button';
        button.onclick = () => {
          fetch("instance", {
            method: "POST",
            headers: {
              "Content-Type": "text/plain"
            },
            body: instance
          });
        };
        instanceContainer.appendChild(button);
      });

      app.appendChild(instanceContainer);
    };

    socket.onclose = () => {
      displayMessage('The WebSocket connection has been closed.');
    };
  </script>
</body>
</html>
)=====";
