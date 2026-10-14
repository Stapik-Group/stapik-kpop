# Backups and restoring

## Automatic copies

Before every save the program keeps the previous version of the collection file. It holds the **five** latest copies,
next to the collection file, named `collection.json.bak`, `collection.json.bak.2` and so on up to `.bak.5`. A damaged
file is never copied, so a broken file cannot push out a good copy.

## Restoring an older version

1. Choose **File → Restore from backup…**.
2. Pick a copy by its date and time.
3. Confirm. The collection is replaced by the chosen version.

The collection from before the restore becomes the newest copy, so you can take the restore back the same way. The
undo history is cleared by a restore. With the cloud connected, the restored version is sent there as well.

> Images are not part of these copies. A restored entry may point to a cover that is no longer on this computer. With
> the cloud connected it is downloaded again.

## Backing up the whole application

**File → Back up the whole application…** saves one ZIP file with the collection, the settings and all covers and
photos. Keep it in a safe place or take it to another computer. **File → Restore the whole application…** reads such a
file and, after asking, replaces the collection and the settings with the ones from it. Images already on this
computer stay, nothing is deleted. The collection from before the restore becomes the newest backup copy, so it can be
taken back as described above. The address and the key of the cloud are not in the file, so passing it around does not
reveal them.

## Where the data is

On Linux the collection is in `~/.local/share/stapikkpop/collection.json`, with the pictures in the `images` and
`photos` folders next to it. On Windows it is in `%APPDATA%\stapikkpop\data`. To make your own backup, copy that folder.

If the collection file is damaged, the program moves it aside (the name gets `.corrupt-` and a date), starts with an
empty collection and tells you. A file saved by a newer version of the program is left untouched and not opened.
