export const registry = {
    entries: new Map(),
    register(entry) {
        this.entries.set(entry.entryName, entry);
    },
    get(entryName) {
        if (!this.entries.has(entryName)) {
            throw new Error(`Entry not found: ${entryName}`);
        }
        return this.entries.get(entryName);
    }
};

const wait = (ms) => new Promise(resolve => setTimeout(resolve, ms));

registry.register({
    entryName: "MagneticField.Initialize",
    requiredUsage: {
        purpose: "최초의 자기장 중심점과 최대 반경, 에이전트들의 초기 위치를 설정한다.",
        preconditions: ["시뮬레이션 상태가 초기화되어 있어야 한다."],
        inputs: ["mapSize: 맵의 전체 크기", "agentCount: 스폰할 에이전트 수"],
        outputs: ["초기 자기장 영역 (중심점, 반경)", "생성된 에이전트 배열"],
        sideEffects: ["화면에 맵 경계, 초기 자기장, 에이전트들이 렌더링된다."],
        completionCondition: "모든 에이전트가 맵 내에 배치되고 초기 자기장 시각화가 완료됨.",
        failureConditions: ["맵 크기가 유효하지 않거나 에이전트 수가 0 이하인 경우"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;
        
        // Setup initial agents
        const agentCount = 8;
        state.agents = [];
        for (let i = 0; i < agentCount; i++) {
            // Random position across the map
            const x = Math.random() * (state.mapSize - 40) + 20;
            const y = Math.random() * (state.mapSize - 40) + 20;
            state.addAgent(x, y, `Agent_${i}`);
        }

        // Setup initial safe zone
        state.currentCenter = { x: state.mapSize / 2, y: state.mapSize / 2 };
        state.currentRadius = state.mapSize * 0.45;
        state.targetCenter = null;
        state.targetRadius = null;

        state.updateAgentStatus();
        
        // Render and wait briefly for visualization
        renderer.render(state);
        await wait(500);
    }
});

registry.register({
    entryName: "MagneticField.CalculateNextPhase",
    requiredUsage: {
        purpose: "다음 페이즈의 좁아진 자기장 목표 반경과 새로운 중심점을 계산한다.",
        preconditions: ["초기 자기장이 존재해야 한다.", "현재 진행 중인 축소 애니메이션이 없어야 한다."],
        inputs: ["currentRadius: 현재 자기장 반경", "currentCenter: 현재 자기장 중심점", "shrinkFactor: 축소 비율 (예: 0.6)"],
        outputs: ["targetRadius: 다음 페이즈의 목표 반경", "targetCenter: 다음 페이즈의 목표 중심점"],
        sideEffects: ["시각적으로 다음 목표 영역이 점선으로 표시되고, 서서히 현재 자기장이 목표를 향해 축소 애니메이션을 재생한다."],
        completionCondition: "현재 반경과 중심점이 목표 반경과 중심점에 도달하고 애니메이션이 종료됨.",
        failureConditions: ["현재 반경이 이미 최소 허용치 이하인 경우"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;
        
        // Calculate new target (shrink by 50% and move center randomly within old radius - new target radius)
        const shrinkFactor = 0.5;
        state.targetRadius = state.currentRadius * shrinkFactor;

        const maxOffset = state.currentRadius - state.targetRadius;
        const angle = Math.random() * Math.PI * 2;
        const dist = Math.random() * maxOffset;
        
        state.targetCenter = {
            x: state.currentCenter.x + Math.cos(angle) * dist,
            y: state.currentCenter.y + Math.sin(angle) * dist
        };

        renderer.render(state, true);
        await wait(500); // show target first

        // Animate shrinking
        const frames = 60;
        const dx = (state.targetCenter.x - state.currentCenter.x) / frames;
        const dy = (state.targetCenter.y - state.currentCenter.y) / frames;
        const dr = (state.targetRadius - state.currentRadius) / frames;

        for (let i = 0; i < frames; i++) {
            if (runtime.isPaused) {
                await runtime.waitWhilePaused();
            }
            state.currentCenter.x += dx;
            state.currentCenter.y += dy;
            state.currentRadius += dr;
            state.updateAgentStatus();
            renderer.render(state, true);
            await wait(16); // ~60fps
        }

        // Finalize state
        state.currentCenter = { ...state.targetCenter };
        state.currentRadius = state.targetRadius;
        state.targetCenter = null;
        state.targetRadius = null;
        state.updateAgentStatus();
        renderer.render(state);
    }
});

registry.register({
    entryName: "MagneticField.ApplyDamageToOutsideAgents",
    requiredUsage: {
        purpose: "자기장 외곽에 있는 에이전트들을 식별하고 지속적인 틱 데미지를 적용한다.",
        preconditions: ["자기장 축소가 완료되어 고정된 상태여야 한다."],
        inputs: ["agents: 현재 에이전트 배열", "currentCenter: 현재 자기장 중심점", "currentRadius: 현재 자기장 반경", "tickDamage: 틱당 감소할 HP량"],
        outputs: ["변경된 에이전트들의 HP 상태"],
        sideEffects: ["안전구역 밖에 있는 에이전트들의 색상이 붉게 점멸하고 HP 수치가 깎이는 애니메이션이 출력된다. HP가 0이 되면 파괴 이펙트와 함께 제거 표시가 된다."],
        completionCondition: "안전구역 외곽에 있는 모든 에이전트에 한 틱의 데미지가 시각적으로 모두 적용됨.",
        failureConditions: ["에이전트 배열이 비어있는 경우"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;

        let hasOutside = false;
        state.updateAgentStatus();

        for (let agent of state.agents) {
            if (agent.hp <= 0) continue;
            if (agent.isOutside) {
                hasOutside = true;
                agent.justDamaged = true;
                agent.hp -= state.tickDamage;
                if (agent.hp < 0) agent.hp = 0;
            }
        }

        renderer.render(state);

        if (hasOutside) {
            await wait(300); // flash duration
            for (let agent of state.agents) {
                agent.justDamaged = false;
            }
            renderer.render(state);
            await wait(200);
        }
    }
});
