// -------------- SetClassic --------------
function setClassic() {
fetch('/setclassic')
    .catch(err => console.error("Set classic error:", err));
}

// -------------- SetExpansion --------------
function setExpansion() {
fetch('/setexpansion')
    .catch(err => console.error("Set expansion error:", err));
}

// -------------- Board Generator --------------
//Server-provided boardNumbers and resources
let boardNumbersFromServer = [];
let boardResourcesFromServer = [];

// The color layout for each tile (19 total):
const hexColors = [
'light-green', 'light-green', 'light-green', 'light-green',
'grey', 'grey', 'grey',
'brick', 'brick', 'brick',
'yellow', 'yellow', 'yellow', 'yellow',
'dark-green', 'dark-green', 'dark-green', 'dark-green',
'desert'
];

// On page load, generate the board in "placeholder" mode (all '--')
window.addEventListener('load', () => {
generateBoard([]);
});

// Generate/Update Board: fill the 19 hexes with the passed numbers
// If 'nums' is empty or missing, we show "--" for each tile.
function generateBoard(nums) {
const boardDiv = document.getElementById('board');
boardDiv.innerHTML = ''; // Clear

//const rowSizes = [3, 4, 5, 4, 3];
const rowSizes = [3, 4, 5, 6, 5, 4, 3];
let colorIndex = 0;
let numIndex = 0;

for (let row = 0; row < 7; row++) {
    const rowDiv = document.createElement('div');
    rowDiv.classList.add('row');

    for (let i = 0; i < rowSizes[row]; i++) {
    const hex = document.createElement('div');
    hex.classList.add('hex', hexColors[colorIndex]);

    // if it's 'desert' color, we show "--"
    // otherwise display the corresponding number from 'nums'
    let tileNumber = "--";
    if (hexColors[colorIndex] !== 'desert' && nums.length === 19) {
        tileNumber = nums[numIndex];
        numIndex++;
    } else if (hexColors[colorIndex] === 'desert') {
        // If you wanted to treat "desert" as 0, or keep it as "--"
        tileNumber = "--";
    }

    hex.textContent = tileNumber;

    // highlight 6 or 8 in red
    if (tileNumber == 6 || tileNumber == 8) {
        hex.classList.add('red');
    }

    colorIndex++;
    rowDiv.appendChild(hex);
    }
    boardDiv.appendChild(rowDiv);
}
}

// -------------- Modal logic --------------
const settingsModal = document.getElementById("settingsModal");
const openSettingsBtn = document.getElementById("openSettingsBtn");
const closeSettingsBtn = document.getElementById("closeSettingsBtn");

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
// Option 1: 6 & 8 Can Touch
document.getElementById("option1").addEventListener("change", function() {
let value = this.checked ? "1" : "0";
fetch('/updateOption1?value=' + value)
    .catch(err => console.error("Error updating Option 1:", err));
});

// Option 2: 2 & 12 Can Touch
document.getElementById("option2").addEventListener("change", function() {
let value = this.checked ? "1" : "0";
fetch('/updateOption2?value=' + value)
    .catch(err => console.error("Error updating Option 2:", err));
});

// Option 3: Same Numbers Can Touch
document.getElementById("option3").addEventListener("change", function() {
let value = this.checked ? "1" : "0";
fetch('/updateOption3?value=' + value)
    .catch(err => console.error("Error updating Option 3:", err));
});

// Option 4: Same Resource Can Touch
document.getElementById("option4").addEventListener("change", function() {
let value = this.checked ? "1" : "0";
fetch('/updateOption4?value=' + value)
    .catch(err => console.error("Error updating Option 4:", err));
});