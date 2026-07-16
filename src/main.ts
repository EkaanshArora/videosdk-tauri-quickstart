import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";

type Participant = {
  id: string;
  name: string;
  isLocal: boolean;
  audioMuted: boolean;
  videoOn: boolean;
};

type SessionState = {
  status: "ready" | "joining" | "joined" | "error";
  participants: Participant[];
  error: string | null;
};

type VideoView = {
  userId: string;
  x: number;
  y: number;
  width: number;
  height: number;
  scale: number;
};

const joinScreen = document.querySelector<HTMLElement>("#join-screen")!;
const sessionScreen = document.querySelector<HTMLElement>("#session-screen")!;
const joinForm = document.querySelector<HTMLFormElement>("#join-form")!;
const joinButton = document.querySelector<HTMLButtonElement>("#join-button")!;
const errorMessage = document.querySelector<HTMLElement>("#error-message")!;
const sessionTitle = document.querySelector<HTMLElement>("#session-title")!;
const videoGrid = document.querySelector<HTMLElement>("#video-grid")!;
const audioButton = document.querySelector<HTMLButtonElement>("#audio-button")!;
const videoButton = document.querySelector<HTMLButtonElement>("#video-button")!;

let state: SessionState = { status: "ready", participants: [], error: null };
let refreshRunning = false;
let refreshAgain = false;

joinForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  errorMessage.textContent = "";
  joinButton.disabled = true;
  joinButton.textContent = "Joining…";

  const data = new FormData(joinForm);
  sessionTitle.textContent = String(data.get("sessionName"));

  try {
    await invoke("join_session", {
      input: {
        sessionName: data.get("sessionName"),
        userName: data.get("userName"),
        password: data.get("password"),
        token: data.get("token"),
      },
    });
    await refreshState();
  } catch (error) {
    showJoinError(error);
  }
});

audioButton.addEventListener("click", () => runControl("toggle_audio"));
videoButton.addEventListener("click", () => runControl("toggle_video"));
document.querySelector("#leave-button")!.addEventListener("click", async () => {
  try {
    await invoke("leave_session");
    await invoke("sync_video_views", { views: [] });
    showJoinScreen();
  } catch (error) {
    window.alert(String(error));
  }
});

async function runControl(command: string) {
  try {
    await invoke(command);
    await refreshState();
  } catch (error) {
    window.alert(String(error));
  }
}

async function refreshState() {
  // Several Zoom callbacks can arrive together. Collapse them into one extra refresh.
  if (refreshRunning) {
    refreshAgain = true;
    return;
  }

  refreshRunning = true;
  do {
    refreshAgain = false;
    state = await invoke<SessionState>("get_session_state");
    render();
  } while (refreshAgain);
  refreshRunning = false;
}

function render() {
  if (state.status === "joined") {
    joinScreen.hidden = true;
    sessionScreen.hidden = false;
    renderGrid();
    return;
  }

  if (state.status === "error") {
    showJoinError(state.error ?? "Unable to join the session");
  } else if (state.status === "ready" && !sessionScreen.hidden) {
    showJoinScreen();
  }
}

function renderGrid() {
  const users = state.participants;
  const columns = Math.ceil(Math.sqrt(Math.max(users.length, 1)));
  const rows = Math.ceil(Math.max(users.length, 1) / columns);
  videoGrid.style.setProperty("--columns", String(columns));
  videoGrid.style.setProperty("--rows", String(rows));

  videoGrid.replaceChildren(
    ...users.map((user) => {
      const tile = document.createElement("article");
      tile.className = "video-tile";
      tile.dataset.userId = user.id;

      const surface = document.createElement("div");
      surface.className = "video-surface";
      surface.textContent = initials(user.name);

      const label = document.createElement("div");
      label.className = "video-label";
      label.innerHTML = `<span>${escapeHtml(user.name)}${user.isLocal ? " (You)" : ""}</span><span>${user.audioMuted ? "Muted" : ""}</span>`;
      tile.append(surface, label);
      return tile;
    }),
  );

  const me = users.find((user) => user.isLocal);
  audioButton.textContent = me?.audioMuted ? "Unmute" : "Mute";
  videoButton.textContent = me?.videoOn ? "Stop video" : "Start video";
  requestAnimationFrame(syncVideoViews);
}

async function syncVideoViews() {
  if (state.status !== "joined") return;

  const views: VideoView[] = state.participants
    .filter((user) => user.videoOn)
    .map((user) => {
      const surface = document.querySelector<HTMLElement>(`[data-user-id="${CSS.escape(user.id)}"] .video-surface`)!;
      const rect = surface.getBoundingClientRect();
      return {
        userId: user.id,
        x: rect.x,
        y: rect.y,
        width: rect.width,
        height: rect.height,
        scale: window.devicePixelRatio,
      };
    });

  await invoke("sync_video_views", { views });
}

function showJoinError(error: unknown) {
  showJoinScreen();
  errorMessage.textContent = String(error);
}

function showJoinScreen() {
  sessionScreen.hidden = true;
  joinScreen.hidden = false;
  joinButton.disabled = false;
  joinButton.textContent = "Join session";
}

function initials(name: string) {
  return name.trim().split(/\s+/).slice(0, 2).map((part) => part[0]?.toUpperCase()).join("");
}

function escapeHtml(value: string) {
  const element = document.createElement("span");
  element.textContent = value;
  return element.innerHTML;
}

window.addEventListener("resize", () => requestAnimationFrame(syncVideoViews));

async function start() {
  await listen("zoom-state-changed", refreshState);
  try {
    await invoke("initialize_sdk");
    await refreshState();
  } catch (error) {
    showJoinError(error);
  }
}

start();
