// To test against your production backend while retrieving code from 
// a development system, change the second target (/Myth, /Guide, etc.)
// to reference your production backend.

const PROXY_CONFIG = [
    {
        context: [
            "/assets",
            "/3rdParty",
            "/images"
        ],
        target: "http://localhost:6547",
    },
    {
        context: [
            "/Frontend",
        ],
        target: "http://localhost:6547",
    }
]

module.exports = PROXY_CONFIG;
