// Clean text and remove non-alphanumeric characters
function preprocessText(text) {
    return text.toLowerCase().replace(/[^a-z0-9\s]/g, ' ');
}

// Convert text into an array of words
function tokenize(text) {
    const cleaned = preprocessText(text);
    return cleaned.trim().split(/\s+/).filter(word => word.length > 0);
}

// Generate N-Grams (Tri-grams)
function generateNGrams(tokens, n = 3) {
    let ngrams = [];
    if (tokens.length < n) return ngrams;

    for (let i = 0; i <= tokens.length - n; i++) {
        let gram = tokens.slice(i, i + n).join(' ');
        ngrams.push(gram);
    }
    return ngrams;
}

// Calculate Jaccard Similarity Percentage
function calculateJaccardSimilarity(grams1, grams2) {
    if (grams1.length === 0 || grams2.length === 0) return 0.0;

    const set1 = new Set(grams1);
    const set2 = new Set(grams2);

    let intersectionSize = 0;
    set1.forEach(gram => {
        if (set2.has(gram)) {
            intersectionSize++;
        }
    });

    const unionSize = new Set([...set1, ...set2]).size;
    return ((intersectionSize / unionSize) * 100).toFixed(2);
}

// Read text contents from a File object asynchronously
function readFileContent(file) {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = (e) => resolve(e.target.result);
        reader.onerror = (e) => reject(e);
        reader.readAsText(file);
    });
}

// Main execution function triggered by button click
async function runPlagiarismCheck() {
    const targetFileInput = document.getElementById('targetFile');
    const libraryFilesInput = document.getElementById('libraryFiles');

    if (!targetFileInput.files.length) {
        alert("Please select a target file (Step 1).");
        return;
    }

    if (!libraryFilesInput.files.length) {
        alert("Please select a library folder or reference files (Step 2).");
        return;
    }

    const targetFile = targetFileInput.files[0];
    const targetText = await readFileContent(targetFile);
    
    const targetTokens = tokenize(targetText);
    const targetNGrams = generateNGrams(targetTokens, 3);

    let results = [];

    // Compare target file against all files in library
    for (let file of libraryFilesInput.files) {
        // Skip non-txt files or hidden system files
        if (!file.name.endsWith('.txt')) continue;

        const libText = await readFileContent(file);
        const libTokens = tokenize(libText);
        const libNGrams = generateNGrams(libTokens, 3);

        const score = parseFloat(calculateJaccardSimilarity(targetNGrams, libNGrams));

        let status = 'LOW / UNIQUE';
        let statusClass = 'low';

        if (score >= 60.0) {
            status = 'HIGH MATCH';
            statusClass = 'high';
        } else if (score >= 20.0) {
            status = 'MODERATE OVERLAP';
            statusClass = 'moderate';
        }

        results.push({
            fileName: file.name,
            score: score,
            status: status,
            statusClass: statusClass
        });
    }

    // Sort results by highest similarity score
    results.sort((a, b) => b.score - a.score);

    // Display Output Table
    document.getElementById('targetName').innerText = targetFile.name;
    const tableBody = document.getElementById('resultsTable');
    tableBody.innerHTML = '';

    results.forEach(res => {
        const row = `<tr>
            <td>${res.fileName}</td>
            <td>${res.score}%</td>
            <td class="${res.statusClass}">${res.status}</td>
        </tr>`;
        tableBody.innerHTML += row;
    });

    document.getElementById('resultsSection').style.display = 'block';
}