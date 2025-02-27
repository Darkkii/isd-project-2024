"use strict";

let measurements;

async function setContentDependingOnSelectedLang() {
  const selectedLang = getSelectedLang();

  styleLanguagesSvg(selectedLang);
  setHeading(selectedLang);
  await setTableContent(selectedLang);
}

function getSelectedLang() {
  const urlParams = new URLSearchParams(window.location.search);
  const langFromQuery = urlParams.get("lang");
  const queryLangIsValid =
    langFromQuery === "ger" ||
    langFromQuery === "fin" ||
    langFromQuery === "en";
  return queryLangIsValid ? langFromQuery : "en";
}

function styleLanguagesSvg(selectedLang) {
  const idOfElementWithFullOpacity = selectedLang + "-flag";
  document.getElementById(idOfElementWithFullOpacity).style.opacity = 1;

  const notSelectedLanguages = isGerman(selectedLang)
    ? ["fin", "en"]
    : isEnglish(selectedLang)
    ? ["fin", "ger"]
    : ["en", "ger"];
  notSelectedLanguages.forEach((notSelectedLang) => {
    const idOfElementWithLimitedlOpacity = notSelectedLang + "-flag";
    document.getElementById(idOfElementWithLimitedlOpacity).style.opacity = 0.4;
  });
}

function setHeading(selectedLang) {
  document.getElementById("header").textContent = isGerman(selectedLang)
    ? "Internationale Sensorentwicklung 2024/2025"
    : isFinnish(selectedLang)
    ? "Kansainvälinen anturikehitys 2024/2025"
    : "International Sensor Development 2024/2025";
}

async function setTableContent(selectedLang) {
  let tableContent = `
        <tr>
            <th style="width: 50%">${
              isGerman(selectedLang) || isEnglish(selectedLang)
                ? "Name"
                : "Nimi"
            }</th>
            <th>${
              isGerman(selectedLang)
                ? "Wert"
                : isFinnish(selectedLang)
                ? "Sisältö"
                : "Value"
            }</th>
        </tr>
    `;

  fetch("/data.json")
    .then((res) => res.json())
    .then((data) => {
      if (!data.length) {
        return;
      }

      measurements = data;

      measurements.forEach((measurement) => {
        tableContent += `
      <tr>
          <td>${
            isGerman(selectedLang)
              ? measurement.name.ger
              : isFinnish(selectedLang)
              ? measurement.name.fin
              : measurement.name.en
          }</td>
           <td>${
            isNaN(parseFloat(measurement.value))
                ? ""
                : parseFloat(measurement.value).toFixed(2)
        } ${measurement.unit}</td>
      </tr>
      `;
      });
    })
    .finally(() => {
      document.getElementById("table").innerHTML = tableContent;
    });

  //   measurements.forEach((measurement) => {
  //     tableContent += `
  //     <tr>
  //         <td>${
  //           isGerman(selectedLang)
  //             ? measurement.name.ger
  //             : isFinnish(selectedLang)
  //             ? measurement.name.fin
  //             : measurement.name.en
  //         }</td>
  //         <td>${measurement.value} ${measurement.unit}</td>
  //     </tr>
  // `;
  //   });
}

function registerEventListeners() {
  languages.forEach((lang) => {
    document.getElementById(lang + "-flag").addEventListener("click", () => {
      reloadWithQueryParamFor(lang);
    });
  });

  document.getElementById("download-svg").addEventListener("click", () => {
    downloadCsv();
  });
}

function reloadWithQueryParamFor(selectedLang) {
  const url = new URL(window.location.href);
  url.searchParams.set("lang", selectedLang);
  window.location.href = url.toString();
}

function downloadCsv() {
  let csvContent = "data:text/csv;charset=utf-8,";

  csvContent += "Name (FIN),Name (GER), Name (EN), Value,Unit" + "\r\n";
  measurements.forEach(function (measurement) {
    let skipRow = measurement.value === "" || measurement.unit === "";
    if(skipRow) {
      return;
    }
    let row = measurement.name.fin + "," + measurement.name.ger + "," + measurement.name.en + "," + measurement.value + "," + measurement.unit
    csvContent += row + "\r\n";
  });
  window.open(encodeURI(csvContent));
}

function setPageReload() {
  setTimeout(() => {
    location.reload();
  }, 10000);
}

function isGerman(lang) {
  return lang === "ger";
}

function isEnglish(lang) {
  return lang === "en";
}

function isFinnish(lang) {
  return lang === "fin";
}

const languages = ["ger", "fin", "en"];
// const measurements = [
//   {
//     name: {
//       ger: "Temperatur",
//       fin: "Lämpötila",
//       en: "Temperature",
//     },
//     value: "20",
//     unit: "°C",
//   },
//   {
//     name: {
//       ger: "Luftfeuchtigkeit",
//       fin: "Ilmankosteus",
//       en: "Humidity",
//     },
//     value: "45",
//     unit: "%",
//   },
//   {
//     name: {
//       ger: "Luftdruck",
//       fin: "Ilmanpaine",
//       en: "Pressure",
//     },
//     value: "1013",
//     unit: "hPa",
//   },
// ];

setContentDependingOnSelectedLang();
registerEventListeners();
setPageReload();
