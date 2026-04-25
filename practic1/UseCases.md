# Use Cases (Step 6)

## UC-01: Registration -> Login -> Account View (implemented)

### Goal
User creates an account, logs in, and sees account data.

### Input data
- Registration: `username`, `email`, `password`
- Login: `email`, `password`

### Sequence
1. User enters registration data.
2. `AuthService::RegisterUser(...)` validates data:
   - non-empty username/password
   - basic email format
   - unique email in storage
3. Service calls `UserStorage::Add(...)`.
4. `UserStorage` hashes password and saves user record into `users_app.dat`.
5. User enters login data.
6. `AuthService::LoginByEmail(...)` finds user by email.
7. Service validates password hash and user state (`is_blocked == 0`).
8. On success, app prints account information.

### Expected result
- New user is persisted in file.
- Login succeeds for correct credentials.
- Account data is returned and displayed.

### Functions used
- Service:
  - `AuthService::RegisterUser(...)`
  - `AuthService::LoginByEmail(...)`
- Storage:
  - `UserStorage::Add(...)`
  - `UserStorage::ReadAll(...)`
- Security:
  - `security::HashPassword(...)`
  - `security::VerifyPassword(...)`

## Additional user actions (planned)

### UC-02: Upload track
- Input: `author_id`, `title`, `file_path`, `genre`, `duration`, `bpm`
- Result: track saved into `tracks.dat`
- Functions: `TrackService::UploadTrack(...)`, `TrackStorage::Add(...)`

### UC-03: Get random track
- Input: `viewer_user_id`
- Result: one active random track
- Functions: `TrackService::GetRandomTrackForUser(...)`, `TrackStorage::ReadAll(...)`

### UC-04: Rate track
- Input: `user_id`, `track_id`, `value(0..5)`
- Result: rating saved/updated, track stats recalculated
- Functions: `RatingService::RateTrack(...)`, `RatingStorage`, `TrackStorage`
