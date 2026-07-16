import { createHmac } from "node:crypto";

const [sessionName, role = "host"] = Bun.argv.slice(2);
const sdkKey = process.env.ZOOM_VIDEO_SDK_KEY ?? process.env.SDK_KEY;
const sdkSecret = process.env.ZOOM_VIDEO_SDK_SECRET ?? process.env.SDK_SECRET;

if (!sessionName || !sdkKey || !sdkSecret || !["host", "participant"].includes(role)) {
  console.error('Usage: bun run token "Session Name" [host|participant]');
  console.error("Add SDK_KEY and SDK_SECRET to .env first.");
  process.exit(1);
}

const now = Math.floor(Date.now() / 1000);
const payload = {
  app_key: sdkKey,
  tpc: sessionName,
  role_type: role === "host" ? 1 : 0,
  version: 1,
  iat: now - 30,
  exp: now + 2 * 60 * 60,
};

const encode = (value: object) => Buffer.from(JSON.stringify(value)).toString("base64url");
const unsignedToken = `${encode({ alg: "HS256", typ: "JWT" })}.${encode(payload)}`;
const signature = createHmac("sha256", sdkSecret).update(unsignedToken).digest("base64url");

console.log(`${unsignedToken}.${signature}`);
