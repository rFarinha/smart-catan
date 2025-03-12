// Global state variables
let gameStarted = false;            // Tracks if game is currently running
let extension = false;              // Tracks if extension board mode is active
let eight_six_canTouch = true;      // Setting: Can 6 and 8 be adjacent?
let two_twelve_canTouch = true;     // Setting: Can 2 and 12 be adjacent?
let sameNumbers_canTouch = true;    // Setting: Can same numbers be adjacent?
let sameResource_canTouch = true;   // Setting: Can same resources be adjacent?
let manualDice = true;              // Setting: Can players manually select dice values?

let currentSelectedNumber = 0;      // Currently selected number token (0 means none)

// DOM Element references (populated in window.load)
var numberButtons,
  diceRollButton,
  classicBtn,
  extensionBtn,
  openSettingsBtn,
  option1,
  option2,
  option3,
  option4,
  option5,
  settingsModa,
  closeSettingsBtn;

// -------------- Communication with Server --------------

/**
 * Periodically fetch board state from server to keep UI in sync
 * This runs every second to poll the server for any changes
 */
setInterval(() => {
  fetch('/getboard')
    .then(response => response.json())
    .then(data => {
      // Update local state with server data
      gameStarted = data.gameStarted
      extension = data.extension;
      currentSelectedNumber = data.selectedNumber;
      manualDice = data.manualDice;
      updateStates(data);
    })
    .catch(err => console.error("Error fetching board:", err));
}, 1000);

/**
 * Update UI elements based on server data
 * @param {Object} data - Board state data from server
 */
function updateStates(data) {
  // Update the board with the server's response
  generateBoard(data);

  // Update mode button text based on the board's mode
  if (extension) {
    classicBtn.textContent = "Classic";
    extensionBtn.textContent = "Shuffle";
  } else {
    classicBtn.textContent = "Shuffle";
    extensionBtn.textContent = "Extension";
  }

  // Handle UI changes based on game state (started/not started)
  if (gameStarted) {
    // Show number buttons and disable mode/options when game is active
    if (manualDice) {
      numberButtons.style.display = "grid";
    }
    diceRollButton.style.display = "block";
    classicBtn.disabled = true;
    extensionBtn.disabled = true;
    openSettingsBtn.disabled = true;
    option1.disabled = true;
    option2.disabled = true;
    option3.disabled = true;
    option4.disabled = true;
    option5.disabled = true;
    startGameBtn.textContent = "End Game";
  } else {
    // Hide number buttons and re-enable mode/options when game is not active
    numberButtons.style.display = "none";
    diceRollButton.style.display = "none";
    classicBtn.disabled = false;
    extensionBtn.disabled = false;
    openSettingsBtn.disabled = false;
    option1.disabled = false;
    option2.disabled = false;
    option3.disabled = false;
    option4.disabled = false;
    option5.disabled = false;
    startGameBtn.textContent = "Start Game";
  }

  // Sync toggle switches with server state
  option1.checked = data.eightSixCanTouch;
  option2.checked = data.twoTwelveCanTouch;
  option3.checked = data.sameNumbersCanTouch;
  option4.checked = data.sameResourceCanTouch;
  option5.checked = data.manualDice;

  // Update visual appearance of board elements
  updateBoardColors(currentSelectedNumber);
  updateNumberButtonColors(currentSelectedNumber);
}

// -------------- Board Mode Selection --------------

/**
 * Set board to classic mode or shuffle existing classic board
 */
function setClassic() {
  fetch('/setclassic')
    .then(response => response.json())
    .then(data => {
      if (!data.extension) {
        extension = false;
        updateStates(data);
      }
    })
    .catch(err => console.error("Set classic error:", err));
}

/**
 * Set board to extension mode or shuffle existing extension board
 */
function setExtension() {
  fetch('/setextension')
    .then(response => response.json())
    .then(data => {
      if (data.extension) {
        extension = true;
        updateStates(data);
      }
    })
    .catch(err => console.error("Set extension error:", err));
}

// -------------- Board Generation --------------

/**
 * Generate the visual board based on server data
 * @param {Object} boardData - Server data containing board state
 * boardData should include:
 *   - resources: array of resource IDs
 *   - numbers: array of token values (with desert hexes as 0)
 *   - extension: boolean indicating board mode
 */
function generateBoard(boardData) {
  const boardDiv = document.getElementById('board');
  boardDiv.innerHTML = ''; // Clear any existing content

  // Determine row sizes based on board mode
  const rowSizes = boardData.extension
    ? [4, 5, 6, 6, 5, 4]  // Extension board layout
    : [3, 4, 5, 4, 3];    // Classic board layout

  let hexIndex = 0;
  // Loop through each row
  for (let row = 0; row < rowSizes.length; row++) {
    const rowDiv = document.createElement('div');
    rowDiv.classList.add('row');

    // Apply offset to create proper hexagonal grid layout
    if (boardData.extension && row < 3) {
      rowDiv.classList.add('offsetLeft');
    }
    else if (boardData.extension && row >= 3) {
      rowDiv.classList.add('offsetRight');
    }

    // Loop through each hex in the row
    for (let col = 0; col < rowSizes[row]; col++) {
      // Ensure we don't overrun the arrays
      if (hexIndex >= boardData.resources.length) break;

      const resourceId = boardData.resources[hexIndex];
      const token = boardData.numbers[hexIndex];

      const hex = document.createElement('div');
      // Add classes for the hex shape and its resource color
      hex.classList.add('hex', resourceToClass(resourceId));

      // If the hex is a desert, display '--'; otherwise, display the token
      hex.textContent = (resourceId === 5 ? '--' : token);

      // Check if this token should be red or black based on the current selected number
      if (currentSelectedNumber && token === currentSelectedNumber) {
        hex.classList.add('red');
      } else {
        hex.classList.add('black');
      }

      rowDiv.appendChild(hex);
      hexIndex++;
    }
    boardDiv.appendChild(rowDiv);
  }
}

/**
 * Maps a resource ID to its corresponding CSS class
 * @param {number} resourceId - Resource ID from server
 * @returns {string} CSS class name for the resource
 */
function resourceToClass(resourceId) {
  switch (resourceId) {
    case 0: return 'sheep';   // light-green
    case 1: return 'wood';    // dark-green
    case 2: return 'wheat';   // yellow
    case 3: return 'brick';   // red
    case 4: return 'ore';     // grey
    case 5: return 'desert';  // desert color
    default: return '';
  }
}

// -------------- Game State Management --------------

/**
 * Toggle between starting and ending the game
 */
function startGame() {
  if (!gameStarted) {
    // Start Game: call the server's /startgame endpoint
    fetch('/startgame')
      .then(response => response.json())
      .then(data => {
        gameStarted = data.gameStarted;
        updateStates(data);
      })
      .catch(err => console.error("Error starting game:", err));
  } else {
    // End Game: call the server's /endgame endpoint
    fetch('/endgame')
      .then(response => response.json())
      .then(data => {
        gameStarted = data.gameStarted;
        currentSelectedNumber = data.selectedNumber;
        updateStates(data);
      })
      .catch(err => console.error("Error ending game:", err));
  }
}

// -------------- Number Selection --------------

/**
 * Select a number token during gameplay
 * @param {number} n - Number value to select (2-12)
 */
function selectNumber(n) {
  fetch('/selectNumber?value=' + n)
    .then(response => response.text())
    .then(data => {
      console.log("Server responded:", data);
      currentSelectedNumber = Number(data);
      updateBoardColors(currentSelectedNumber);
      updateNumberButtonColors(currentSelectedNumber);
    })
    .catch(err => console.error("Error sending number:", err));
}

// -------------- Dice Rolling --------------

/**
 * Simulate rolling dice during gameplay
 */
function rollDice() {
  fetch('/rollDice')
    .then(response => response.text())
    .then(data => {
      console.log("Server responded:", data);
      currentSelectedNumber = Number(data);
      updateBoardColors(currentSelectedNumber);
      updateNumberButtonColors(currentSelectedNumber);
    })
    .catch(err => console.error("Error sending number:", err));
}

// -------------- Page Initialization --------------

/**
 * Initialize page on load
 */
window.addEventListener('load', () => {
  loadElementValues();
  addSettingsListeners();
});

/**
 * Load references to DOM elements
 */
function loadElementValues() {
  // Get references to all DOM elements we need to interact with
  startGameBtn = document.querySelector(".start-game-btn");
  numberButtons = document.getElementById("numberButtons");
  diceRollButton = document.getElementById("diceRollButton");
  classicBtn = document.getElementById("classicBtn");
  extensionBtn = document.getElementById("extensionBtn");
  openSettingsBtn = document.getElementById("openSettingsBtn");
  option1 = document.getElementById("option1");
  option2 = document.getElementById("option2");
  option3 = document.getElementById("option3");
  option4 = document.getElementById("option4");
  option5 = document.getElementById("option5");
  settingsModal = document.getElementById("settingsModal");
  closeSettingsBtn = document.getElementById("closeSettingsBtn");
}

// -------------- Modal Handling --------------

/**
 * Add event listeners for settings modal
 */
function addSettingsListeners() {
  // Open settings modal
  openSettingsBtn.addEventListener("click", () => {
    settingsModal.style.display = "block";
  });

  // Close settings modal via button
  closeSettingsBtn.addEventListener("click", () => {
    settingsModal.style.display = "none";
  });

  // Close settings modal by clicking outside
  window.addEventListener("click", (event) => {
    if (event.target == settingsModal) {
      settingsModal.style.display = "none";
    }
  });

  // -------------- Settings Toggle Handlers --------------

  // 6&8 adjacency toggle
  option1.addEventListener("change", function () {
    let value = this.checked ? "1" : "0";
    fetch('/eightSixCanTouch?value=' + value)
      .catch(err => console.error("Error updating eightSixCanTouch:", err));
  });

  // 2&12 adjacency toggle
  option2.addEventListener("change", function () {
    let value = this.checked ? "1" : "0";
    fetch('/twoTwelveCanTouch?value=' + value)
      .catch(err => console.error("Error updating twoTwelveCanTouch:", err));
  });

  // Same numbers adjacency toggle
  option3.addEventListener("change", function () {
    let value = this.checked ? "1" : "0";
    fetch('/sameNumbersCanTouch?value=' + value)
      .catch(err => console.error("Error updating sameNumbersCanTouch:", err));
  });

  // Same resources adjacency toggle
  option4.addEventListener("change", function () {
    let value = this.checked ? "1" : "0";
    fetch('/sameResourceCanTouch?value=' + value)
      .catch(err => console.error("Error updating sameResourceCanTouch:", err));
  });

  // Manual dice toggle
  option5.addEventListener("change", function () {
    let value = this.checked ? "1" : "0";
    fetch('/manualDice?value=' + value)
      .catch(err => console.error("Error updating manualDice:", err));
  });
}

/**
 * Update visual appearance of board hexes based on selected number
 * @param {number} selectedNumber - The currently selected number (2-12, or 7 for robber)
 */
function updateBoardColors(selectedNumber) {
  // Select all hex elements on your board
  const hexes = document.querySelectorAll('.hex');

  hexes.forEach(hex => {
    if (selectedNumber === 7) {
      // For robber (7), highlight all hexes in red
      hex.classList.add("red");
      hex.classList.remove("black");
    } else {
      // Compare the hex's number with the selected number
      if (hex.textContent.trim() === String(selectedNumber)) {
        // Highlight matching hexes in red
        hex.classList.add("red");
        hex.classList.remove("black");
      } else {
        // Set non-matching hexes to black
        hex.classList.remove("red");
        hex.classList.add("black");
      }
    }
  });
}

/**
 * Update appearance of number buttons based on selected number
 * @param {number} selectedNumber - The currently selected number (2-12, or 7 for robber)
 */
function updateNumberButtonColors(selectedNumber) {
  // Update regular number buttons (2-6, 8-12)
  const smallButtons = document.querySelectorAll("#numberButtonsRows button");
  smallButtons.forEach(btn => {
    if (btn.textContent.trim() === String(selectedNumber)) {
      btn.classList.add("red");
      btn.classList.remove("black");
    } else {
      btn.classList.remove("red");
      btn.classList.add("black");
    }
  });

  // Update the big 7 button separately
  const bigSeven = document.getElementById("bigSeven");
  if (selectedNumber === 7) {
    bigSeven.classList.add("red");
    bigSeven.classList.remove("black");
  } else {
    bigSeven.classList.remove("red");
    bigSeven.classList.add("black");
  }
}