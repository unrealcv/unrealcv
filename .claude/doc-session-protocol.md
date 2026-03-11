# Documentation Session Protocol

Follow this sequence every time you start a documentation session.

## 1. Environment Setup (Required)
```bash
cd G:/HUAWEI_Project_UE56/Plugins/unrealcv
./init.bat
```

## 2. Review State
```bash
# Read progress file
cat claude-progress.txt

# Read feature list
cat unrealcv-doc-features.json | jq '.features[] | select(.status != "completed") | {id, title, status}'
```

## 3. Select Next Feature
Find the highest-priority feature with `status == "pending"` and no blocking issues:
```bash
# Quick selection
jq -r '.features | sort_by(.priority, .id) | .[] | select(.status == "pending" and (.blocking | length) == 0) | "[DOC-\(.id)] \(.title)"' unrealcv-doc-features.json | head -1
```

## 4. Study Code
For selected feature:
1. Read all `code_locations` files
2. Extract relevant patterns
3. Note any linked features in `blocking`

## 5. Write Documentation
1. Read existing `doc_locations` (if they exist)
2. Create new file or update existing
3. Follow naming conventions:
   - `docs/reference/` - API references
   - `docs/guides/` - How-to guides
   - `docs/tutorials/` - Step-by-step tutorials
   - `docs/architecture/` - System design docs
   - `docs/diagrams/` - ASCII/mermaid diagrams

## 6. Verification
- [ ] Code locations all reviewed
- [ ] Documentation renders (RST/Markdown)
- [ ] Examples compile/testable
- [ ] Links verified (internal and external)
- [ ] No placeholder text remains

## 7. Commit & Update Progress
```bash
# Stage changes
git add docs/ unrealcv-doc-features.json

# Commit with feature ID
git commit -m "[DOC-<ID>] <title>

- Updated/created: <file changes>
- Verified: <what was tested>

🤖 Generated with Claude Code"

# Update feature status in unrealcv-doc-features.json
# Set status to "completed" with completion date
```

## 8. Update Progress File
Append to `claude-progress.txt`:
```
## <YYYY-MM-DD> - Feature DOC-<ID>
- Completed: <title>
- Next feature: DOC-<NEXT>
```

## Feature Status Types
- `pending` - Not started
- `in_progress` - Currently working
- `blocked` - Waiting on other features
- `completed` - Finished and verified

## Priority Order
1. `critical` - Blockers for other docs
2. `high` - Core functionality
3. `medium` - Enhancement docs
4. `low` - Nice to have
