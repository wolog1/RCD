# Security Policy

## Sensitive information policy

Do not commit production credentials, customer data, device identifiers, private protocols, topology data, database dumps, logs, certificates, or private keys.

All runtime secrets must be injected through environment variables or an external secret-management service. Repository configuration files must contain placeholders only.

## Immediate response to a leaked secret

1. Revoke or rotate the credential immediately.
2. Preserve relevant audit logs.
3. Remove the secret from the current tree and Git history.
4. Review access logs and dependent systems.
5. Document the root cause and preventive control.

Deleting a secret from the latest file version does not invalidate copies in Git history.

## Reporting

Report suspected vulnerabilities privately to the repository owner. Do not place credentials, customer information, exploit details, or production endpoints in public issues.

## Required review controls

- No hard-coded passwords, tokens, AccessKeys, device secrets, or private keys.
- No real customer, station, gateway, meter, or topology data in tests.
- Use least-privilege credentials and separate development from production.
- Security-sensitive changes require manual review before merge.
