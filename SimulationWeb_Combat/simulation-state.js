export class SimulationState {
    constructor(width, height) {
        this.width = width;
        this.height = height;

        this.character = {
            x: width / 2,
            y: height / 2 + 100, // start slightly bottom
            rotation: -Math.PI / 2, // facing up
            radius: 15
        };

        // For Combat.CalculateMovement
        this.moveVector = null;
        
        // For Combat.AimAtCursor
        this.cursorPos = null;
        this.showAimLine = false;

        // For Combat.ShootProjectile
        this.projectiles = [];
        this.targets = [
            { id: 1, x: width / 2 - 100, y: height / 2 - 100, radius: 15, hp: 100, isHit: false },
            { id: 2, x: width / 2 + 100, y: height / 2 - 100, radius: 15, hp: 100, isHit: false },
            { id: 3, x: width / 2, y: height / 2 - 150, radius: 15, hp: 100, isHit: false }
        ];
    }
}
