// Global flag for game state.
let gameStarted = false;
let expansion = false;
let eight_six_canTouch = true;
let two_twelve_canTouch = true;
let sameNumbers_canTouch = true;
let sameResource_canTouch = true;

let currentSelectedNumber = 0;

// Elements
var numberButtons,
  diceRollButton,
  classicBtn,
  expansionBtn,
  openSettingsBtn,
  option1,
  option2,
  option3,
  option4,
  settingsModa,
  closeSettingsBtn

// -------------- Get Board from Server --------------
setInterval(() => {
  fetch('/getboard')
    .then(response => response.json())
    .then(data => {
      gameStarted = data.gameStarted
      expansion = data.expansion;
      currentSelectedNumber = data.selectedNumber;
      updateStates(data);
    })
    .catch(err => console.error("Error fetching board:", err));
}, 1000);

function updateStates(data) {
  
  // Update the board with the server's response.
  generateBoard(data);

    // Update mode button labels based on the board's mode.
  if (expansion) {
    classicBtn.textContent = "Classic";
    expansionBtn.textContent = "Shuffle";
  } else {
    classicBtn.textContent = "Shuffle";
    expansionBtn.textContent = "Expansion";
  }
  // Disable buttons if game as started
  if (gameStarted) {
    // Show number buttons and disable mode/options.
    numberButtons.style.display = "block";
    diceRollButton.style.display = "block";
    classicBtn.disabled = true;
    expansionBtn.disabled = true;
    openSettingsBtn.disabled = true;
    option1.disabled = true;
    option2.disabled = true;
    option3.disabled = true;
    option4.disabled = true;
    startGameBtn.textContent = "End Game";
  } else {
    // Hide number buttons and re-enable mode/options.
    numberButtons.style.display = "none";
    diceRollButton.style.display = "none";
    classicBtn.disabled = false;
    expansionBtn.disabled = false;
    openSettingsBtn.disabled = false;
    option1.disabled = false;
    option2.disabled = false;
    option3.disabled = false;
    option4.disabled = false;
    startGameBtn.textContent = "Start Game";
  }
  option1.checked = data.eightSixCanTouch;
  option2.checked = data.twoTwelveCanTouch;
  option3.checked = data.sameNumbersCanTouch;
  option4.checked = data.sameResourceCanTouch; 

  updateBoardColors(currentSelectedNumber);
}

// -------------- Set Classic --------------
function setClassic() {
  fetch('/setclassic')
    .then(response => response.json())
    .then(data => {
      if (!data.expansion) {
        expansion = false;
        updateStates(data);
      }
    })
    .catch(err => console.error("Set classic error:", err));
}

// -------------- Set Expansion --------------
function setExpansion() {
  fetch('/setexpansion')
    .then(response => response.json())
    .then(data => {
      if (data.expansion) {
        expansion = true;
        updateStates(data);
      }
    })
    .catch(err => console.error("Set expansion error:", err));
}

// -------------- Generate Board --------------
// boardData should be an object with properties:
//   resources: array of resource IDs,
//   numbers: array of tokens (with desert hexes marked as 0 or similar),
//   expansion: a boolean (true if expansion board, false if classic)
function generateBoard(boardData) {
  const boardDiv = document.getElementById('board');
  boardDiv.innerHTML = ''; // Clear any existing content

  // Determine row sizes based on board mode.
  const rowSizes = boardData.expansion 
    ? [4, 5, 6, 6, 5, 4] 
    : [3, 4, 5, 4, 3];

  let hexIndex = 0;
  // Loop through each row.
  for (let row = 0; row < rowSizes.length; row++) {
    const rowDiv = document.createElement('div');
    rowDiv.classList.add('row');

    // For expansion mode, if this is the second 6-row (row index 3), add the offset.
    // For expansion mode, if this is the second 6-row (row index 3), add the offset.
    if (boardData.expansion && row < 3) {
      rowDiv.classList.add('offsetLeft');
    }
    else if (boardData.expansion && row >= 3) {
      rowDiv.classList.add('offsetRight');
    }

    // Loop through each hex in the row.
    for (let col = 0; col < rowSizes[row]; col++) {
      // Ensure we don't overrun the arrays.
      if (hexIndex >= boardData.resources.length) break;
      
      const resourceId = boardData.resources[hexIndex];
      const token = boardData.numbers[hexIndex];
      
      const hex = document.createElement('div');
      // Add classes for the hex shape and its resource color.
      hex.classList.add('hex', resourceToClass(resourceId));

      // If the hex is a desert, display '--'; otherwise, display the token.
      hex.textContent = (resourceId === 5 ? '--' : token);

            // Check if this token should be red or black based on the current selected number.
      if (currentSelectedNumber && token === currentSelectedNumber) {
        hex.classList.add('red');
      } else {
        hex.classList.add('black');
      }

      // Optionally, add a red highlight if the token is 6 or 8 (and not a desert).
      //if ((token === 6 || token === 8) && resourceId !== 5) {
      //  hex.classList.add('red');
      //}

      rowDiv.appendChild(hex);
      hexIndex++;
    }
    boardDiv.appendChild(rowDiv);
  }
}

// Helper function to map a resource ID to a CSS class.
function resourceToClass(resourceId) {
  switch(resourceId) {
    case 0: return 'sheep';   // light-green
    case 1: return 'wood';    // dark-green
    case 2: return 'wheat';   // yellow
    case 3: return 'brick';   // red
    case 4: return 'ore';     // grey
    case 5: return 'desert';  // desert color
    default: return '';
  }
}

// -------------- Start/End Game Toggle --------------
function startGame() {
  if (!gameStarted) {
    // Start Game: call the server's /startgame endpoint.
    fetch('/startgame')
      .then(response => response.json())
      .then(data => {
        gameStarted = data.gameStarted;
        updateStates(data);
      })
      .catch(err => console.error("Error starting game:", err));
  } else {
    // End Game: call the server's /endgame endpoint.
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

// -------------- Number Button Handler --------------
function selectNumber(n) {
  fetch('/selectNumber?value=' + n)
    .then(response => response.text())
    .then(data => {
      console.log("Server responded:", data);
      currentSelectedNumber = Number(data);
      updateBoardColors(currentSelectedNumber);
    })
    .catch(err => console.error("Error sending number:", err));
}

// -------------- Roll Dice --------------------------

function rollDice() {
    fetch('/rollDice')
    .then(response => response.text())
    .then(data => {
      console.log("Server responded:", data);
       currentSelectedNumber = Number(data);
       updateBoardColors(currentSelectedNumber);
    })
    .catch(err => console.error("Error sending number:", err));
}

// -------------- Page Load Logic --------------
// On page load, fetch the current board info from the server.
window.addEventListener('load', () => {
  loadElementValues();
  addSettingsListeners();
});

function loadElementValues() {
  // Elements
  startGameBtn = document.querySelector(".start-game-btn");
  numberButtons = document.getElementById("numberButtons");
  diceRollButton = document.getElementById("diceRollButton");
  classicBtn = document.getElementById("classicBtn");
  expansionBtn = document.getElementById("expansionBtn");
  openSettingsBtn = document.getElementById("openSettingsBtn");
  option1 = document.getElementById("option1");
  option2 = document.getElementById("option2");
  option3 = document.getElementById("option3");
  option4 = document.getElementById("option4");
  settingsModal = document.getElementById("settingsModal");
  closeSettingsBtn = document.getElementById("closeSettingsBtn");
}

// -------------- Modal Logic --------------

function addSettingsListeners() {
  openSettingsBtn.addEventListener("click", () => {
    settingsModal.style.display = "block";
  });
  closeSettingsBtn.addEventListener("click", () => {
    settingsModal.style.display = "none";
  });
  window.addEventListener("click", (event) => {
    if (event.target == settingsModal) {
      settingsModal.style.display = "none";
    }
  });

  // -------------- Settings Sliders Event Listeners --------------
  option1.addEventListener("change", function() {
    let value = this.checked ? "1" : "0";
    fetch('/eightSixCanTouch?value=' + value)
      .catch(err => console.error("Error updating eightSixCanTouch:", err));
  });

  option2.addEventListener("change", function() {
    let value = this.checked ? "1" : "0";
    fetch('/twoTwelveCanTouch?value=' + value)
      .catch(err => console.error("Error updating twoTwelveCanTouch:", err));
  });

  option3.addEventListener("change", function() {
    let value = this.checked ? "1" : "0";
    fetch('/sameNumbersCanTouch?value=' + value)
      .catch(err => console.error("Error updating sameNumbersCanTouch:", err));
  });

  option4.addEventListener("change", function() {
    let value = this.checked ? "1" : "0";
    fetch('/sameResourceCanTouch?value=' + value)
      .catch(err => console.error("Error updating sameResourceCanTouch:", err));
  });
}


function updateBoardColors(selectedNumber) {
  // Select all hex elements on your board
  const hexes = document.querySelectorAll('.hex');
  hexes.forEach(hex => {
    if (selectedNumber === 7) {
      hex.classList.add("red");
      hex.classList.remove("black");
    } else {
      // Compare the text content (which shows the number) with the selected number.
      // Trim to remove any extra whitespace.
      if (hex.textContent.trim() === String(selectedNumber)) {
        hex.classList.add("red");
        hex.classList.remove("black");
      } else {
        // Remove the red class and optionally add a black class.
        hex.classList.remove("red");
        hex.classList.add("black");
      }
    }
    
  });
}