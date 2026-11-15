# Centralized College Events Tracker — UI starter

## Run the interactive UI
1. Extract the ZIP.
2. Open `index.html` in Chrome/Edge, or open the folder in VS Code and use Live Server.
3. Try search, category filter, sorting, event details, saving events, and Organizer desk to add an event.

## Files
- `index.html`: page structure
- `style.css`: calm, responsive styling
- `app.js`: browser-side interactions and sample event data
- `main.cpp`: starter C++ Event class example

## Important
This is a frontend prototype, not a finished connected app. Events created in the browser are temporary and reset on refresh. Registration URLs are placeholders (`example.com`) and should be replaced with actual college registration links. The C++ file is currently separate from the web UI.

To connect the UI to C++, the next step is to create a C++ HTTP API (for example with the Crow framework) and have JavaScript fetch event data from it. QR codes can then point to each event's real registration URL.
