package translate

import "testing"

func TestQuitting(t *testing.T) {
    if IsQuitting() {
        t.Errorf("Expected Quitting=CONTINUE, Actual: %d\n", Quitting)
    }
    SetQuitting()
    if !IsQuitting() {
        t.Errorf("Expected Quitting=QUITTING, Actual: %d\n", Quitting)
    }
}
