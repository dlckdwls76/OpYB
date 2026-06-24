export class SimulationRenderer {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.width = this.canvas.width;
        this.height = this.canvas.height;
    }

    render(state, isAnimatingPhase = false) {
        this.ctx.clearRect(0, 0, this.width, this.height);

        // Draw Map Border
        this.ctx.strokeStyle = '#333';
        this.ctx.lineWidth = 2;
        this.ctx.strokeRect(0, 0, this.width, this.height);

        // Draw Magnetic Field (Danger Zone is outside, Safe Zone is inside)
        // Danger zone visualization (shaded outside the circle)
        this.ctx.fillStyle = 'rgba(128, 0, 128, 0.2)'; // Purple tint for toxic gas
        this.ctx.beginPath();
        this.ctx.rect(0, 0, this.width, this.height);
        this.ctx.arc(state.currentCenter.x, state.currentCenter.y, state.currentRadius, 0, Math.PI * 2, true);
        this.ctx.fill();

        // Safe Zone Border
        this.ctx.strokeStyle = '#00ffcc';
        this.ctx.lineWidth = 3;
        this.ctx.beginPath();
        this.ctx.arc(state.currentCenter.x, state.currentCenter.y, state.currentRadius, 0, Math.PI * 2);
        this.ctx.stroke();

        // Target Zone Border (if animating)
        if (state.targetCenter && state.targetRadius) {
            this.ctx.strokeStyle = 'rgba(0, 255, 204, 0.5)';
            this.ctx.setLineDash([5, 10]);
            this.ctx.lineWidth = 2;
            this.ctx.beginPath();
            this.ctx.arc(state.targetCenter.x, state.targetCenter.y, state.targetRadius, 0, Math.PI * 2);
            this.ctx.stroke();
            this.ctx.setLineDash([]); // reset dash
        }

        // Draw Agents
        for (let agent of state.agents) {
            if (agent.hp <= 0) {
                // Draw dead agent
                this.ctx.fillStyle = '#555';
                this.ctx.beginPath();
                this.ctx.arc(agent.x, agent.y, 6, 0, Math.PI * 2);
                this.ctx.fill();
                this.ctx.fillStyle = '#ff4444';
                this.ctx.font = '10px Arial';
                this.ctx.fillText("DEAD", agent.x - 12, agent.y - 10);
                continue;
            }

            // Draw alive agent
            this.ctx.fillStyle = agent.isOutside ? '#ff9999' : '#ffffff';
            if (agent.justDamaged) {
                this.ctx.fillStyle = '#ff0000'; // flash red
            }
            
            this.ctx.beginPath();
            this.ctx.arc(agent.x, agent.y, 8, 0, Math.PI * 2);
            this.ctx.fill();
            this.ctx.strokeStyle = '#000';
            this.ctx.lineWidth = 1;
            this.ctx.stroke();

            // HP Bar
            this.ctx.fillStyle = '#000';
            this.ctx.fillRect(agent.x - 10, agent.y - 15, 20, 4);
            this.ctx.fillStyle = agent.hp > 50 ? '#00ff00' : '#ff0000';
            this.ctx.fillRect(agent.x - 10, agent.y - 15, 20 * (agent.hp / 100), 4);
        }
    }
}
