# Cloud synchronisation

The collection can be kept in step on several computers through your own **Stapik Cloud** (version 1.2.0 or newer).
It is optional: without the cloud the program works the same, and all data stays on your computer.

## Connecting

1. Choose **File → Connect to cloud**.
2. Enter the **API URL** of your cloud and an **API key** created in its panel.
3. Press **Check** to see whether the server answers and accepts the key.
4. Press **Connect**.

Use an address starting with `https://`. With plain `http://` the key and the data are sent without protection.

## What is synchronised

The collection (all entries and artists) and the covers and photos. The state is shown at the bottom of the window.
Changes are sent shortly after you make them, **File → Sync** does it at once. When two computers changed the
collection, the newer version wins.

## Encryption

You may enter a **passphrase for end-to-end encryption**. Then the collection is encrypted on your computer and the
server only sees data it cannot read. Remember:

- Use **the same passphrase on every computer**.
- Without the passphrase the data in the cloud **cannot be recovered**. Nobody can reset it.
- Only the collection is encrypted. **Covers and photos are sent as they are**, because the cloud cannot encrypt files.

## Keys and limits

When your API key is about to expire, the program tells you the date. Create a new key in the cloud panel and enter it
again in **Connect to cloud**. If the cloud asks to slow down, the program waits and tries again by itself.
