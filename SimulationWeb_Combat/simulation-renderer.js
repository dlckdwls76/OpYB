export class SimulationRenderer {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.width = this.canvas.width;
        this.height = this.canvas.height;
    }

    render(state) {
        this.ctx.clearRect(0, 0, this.width, this.height);

        // Grid background for movement visualization
        this.ctx.strokeStyle = '#333';
        this.ctx.lineWidth = 1;
        for (let i = 0; i < this.width; i += 50) {
            this.ctx.beginPath(); this.ctx.moveTo(i, 0); this.ctx.lineTo(i, this.height); this.ctx.stroke();
            this.ctx.beginPath(); this.ctx.moveTo(0, i); this.ctx.lineTo(this.width, i); this.ctx.stroke();
        }

        // Draw Targets (Enemies)
        for (let target of state.targets) {
            if (target.hp <= 0) continue;
            this.ctx.fillStyle = target.isHit ? '#ff0000' : '#882222';
            this.ctx.beginPath();
            this.ctx.arc(target.x, target.y, target.radius, 0, Math.PI * 2);
            this.ctx.fill();
            this.ctx.strokeStyle = '#fff';
            this.ctx.stroke();

            // HP Bar
            this.ctx.fillStyle = '#000';
            this.ctx.fillRect(target.x - 15, target.y - 25, 30, 5);
            this.ctx.fillStyle = '#00ff00';
            this.ctx.fillRect(target.x - 15, target.y - 25, 30 * (target.hp / 100), 5);
        }

        // Draw cursor if active
        if (state.cursorPos) {
            this.ctx.strokeStyle = '#ffcc00';
            this.ctx.beginPath();
            this.ctx.arc(state.cursorPos.x, state.cursorPos.y, 8, 0, Math.PI * 2);
            this.ctx.stroke();
            this.ctx.beginPath();
            this.ctx.moveTo(state.cursorPos.x - 12, state.cursorPos.y);
            this.ctx.lineTo(state.cursorPos.x + 12, state.cursorPos.y);
            this.ctx.stroke();
            this.ctx.beginPath();
            this.ctx.moveTo(state.cursorPos.x, state.cursorPos.y - 12);
            this.ctx.lineTo(state.cursorPos.x, state.cursorPos.y + 12);
            this.ctx.stroke();
        }

        // Draw Aim Line
        if (state.showAimLine && state.cursorPos) {
            this.ctx.strokeStyle = 'rgba(255, 204, 0, 0.4)';
            this.ctx.setLineDash([5, 5]);
            this.ctx.lineWidth = 2;
            this.ctx.beginPath();
            this.ctx.moveTo(state.character.x, state.character.y);
            this.ctx.lineTo(state.cursorPos.x, state.cursorPos.y);
            this.ctx.stroke();
            this.ctx.setLineDash([]);
        }

        // Draw Movement Vector
        if (state.moveVector) {
            this.ctx.strokeStyle = '#00ffcc';
            this.ctx.lineWidth = 3;
            this.ctx.beginPath();
            this.ctx.moveTo(state.character.x, state.character.y);
            this.ctx.lineTo(state.character.x + state.moveVector.x * 50, state.character.y + state.moveVector.y * 50);
            this.ctx.stroke();
            // Arrow head
            this.ctx.fillStyle = '#00ffcc';
            this.ctx.beginPath();
            this.ctx.arc(state.character.x + state.moveVector.x * 50, state.character.y + state.moveVector.y * 50, 4, 0, Math.PI*2);
            this.ctx.fill();
        }

        // Draw Projectiles
        for (let proj of state.projectiles) {
            this.ctx.fillStyle = '#ffff00';
            this.ctx.beginPath();
            this.ctx.arc(proj.x, proj.y, 4, 0, Math.PI * 2);
            this.ctx.fill();
        }

        // Draw Character
        this.ctx.save();
        this.ctx.translate(state.character.x, state.character.y);
        this.ctx.rotate(state.character.rotation);

        // Body
        this.ctx.fillStyle = '#0088ff';
        this.ctx.beginPath();
        this.ctx.arc(0, 0, state.character.radius, 0, Math.PI * 2);
        this.ctx.fill();
        this.ctx.strokeStyle = '#fff';
        this.ctx.lineWidth = 2;
        this.ctx.stroke();

        // Gun barrel indicating direction (pointing right because math uses 0 as right)
        this.ctx.fillStyle = '#999';
        this.ctx.fillRect(0, -4, 25, 8);
        this.ctx.strokeRect(0, -4, 25, 8);
        
        this.ctx.restore();
    }
}
