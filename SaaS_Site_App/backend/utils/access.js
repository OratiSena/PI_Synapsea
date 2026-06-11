function buildReadingScope(request, alias) {
  const conditions = [];
  const params = [];
  const { patientId, deviceId } = request.query || {};

  if (patientId) {
    conditions.push(`${alias}.patient_id = ?`);
    params.push(patientId);
  }
  if (deviceId) {
    conditions.push(`${alias}.device_id = ?`);
    params.push(deviceId);
  }
  if (request.user?.role === "user") {
    conditions.push(`(
      EXISTS (
        SELECT 1 FROM patients scoped_patient
        WHERE scoped_patient.id = ${alias}.patient_id
          AND scoped_patient.user_id = ?
      )
      OR EXISTS (
        SELECT 1 FROM device_assignments scoped_assignment
        WHERE scoped_assignment.device_id = ${alias}.device_id
          AND scoped_assignment.user_id = ?
          AND scoped_assignment.active = TRUE
      )
    )`);
    params.push(request.user.id, request.user.id);
  }

  return {
    sql: conditions.length ? ` WHERE ${conditions.join(" AND ")}` : "",
    params
  };
}

function canUsePatient(user, patient) {
  return user?.role === "admin" || (user?.role === "user" && patient?.userId === user.id);
}

module.exports = { buildReadingScope, canUsePatient };
