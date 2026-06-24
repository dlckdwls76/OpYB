export class SimulationState {
    constructor(mapSize) {
        this.mapSize = mapSize;
        this.currentCenter = { x: mapSize / 2, y: mapSize / 2 };
        this.currentRadius = mapSize / 2 * 0.9;
        
        this.targetCenter = null;
        this.targetRadius = null;

        this.agents = [];
        this.tickDamage = 15;
    }

    addAgent(x, y, id) {
        this.agents.push({
            id: id,
            x: x,
            y: y,
            hp: 100,
            isOutside: false,
            justDamaged: false
        });
    }

    updateAgentStatus() {
        for (let agent of this.agents) {
            if (agent.hp <= 0) continue;
            const dx = agent.x - this.currentCenter.x;
            const dy = agent.y - this.currentCenter.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            agent.isOutside = dist > this.currentRadius;
        }
    }
}
