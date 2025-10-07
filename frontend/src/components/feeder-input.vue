<template>
  <input
    v-model="model"
    v-bind="$attrs"
    :type
    :class="classes"
    :min
    :max
    :pattern
    :placeholder
    @blur="onBlur"
    @keydown="onKeyPress"
  />
</template>

<script setup lang="ts">
import { computed, ref } from "vue";

interface IProps {
  type: "number" | "time";
  min?: string;
  max?: string;
  pattern?: string;
  placeholder?: string;
}

const props = defineProps<IProps>();
const model = defineModel<string | number>();
const isShaking = ref<boolean>(false);

function setShakingAnimation(): void {
  isShaking.value = true;
  setTimeout(() => (isShaking.value = false), 500);
}

function onKeyPress(event: KeyboardEvent): boolean {
  if (event.type !== "number") {
    return false;
  }
  const isDeletingKey =
    event.keyCode == 8 || event.keyCode == 0 || event.keyCode == 13;
  const isDigitKey = event.keyCode >= 48 && event.keyCode <= 57;

  if (isDigitKey || isDeletingKey) {
    return true;
  }
  event.preventDefault();
  return false;
}

function onBlur(): void {
  if (props.type === "number") {
    onNumberInputBlur();
  }

  if (props.type === "time") {
    onTimeInputBlur();
  }
}

function onNumberInputBlur(): void {
  const min = Number(props.min);
  const max = Number(props.max);
  const value = Number(model.value);

  if (value < min || value > max) {
    model.value = (value < min ? min : max).toString();
    setShakingAnimation();
  } else {
    model.value = value.toString();
  }
}

function onTimeInputBlur(): void {
  if (!model.value) {
    model.value = "12:00";
    setShakingAnimation();
  }
}

const classes = computed<Record<string, boolean>>(() => {
  return {
    "input-shaking": isShaking.value,
  };
});
</script>

<style>
/* Chrome, Safari, Edge, Opera */
input::-webkit-outer-spin-button,
input::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}

/* Firefox */
input[type="number"] {
  -moz-appearance: textfield;
}

input {
  padding: 0 16px;
  height: 40px;
  font-size: 12px;
  outline: none;
  box-sizing: border-box;
  border-radius: var(--radius-md);
  border: 1px solid var(--border-light);
  background-color: var(--bg-surface-1);
  color: var(--content-default);
  -webkit-text-fill-color: var(--content-default);
  transition: all 0.2s ease-out;
  font-family: var(--font-family);
}

input[readonly],
input[disabled] {
  border: none;
  background: transparent;
  color: var(--content-default);
  -webkit-text-fill-color: var(--content-default);
  opacity: 1;
}

input::placeholder {
  color: var(--neutral-muted);
  -webkit-text-fill-color: var(--neutral-muted);
}

input:focus {
  border-color: var(--primary-default);
}

.input-shaking {
  animation: shake 0.2s ease-in-out 0s 2;
  border-color: red;
}

@keyframes shake {
  0% {
    transform: translateX(0rem);
  }

  25% {
    transform: translateX(0.5rem);
  }

  75% {
    transform: translateX(-0.5rem);
  }

  100% {
    transform: translateX(0rem);
  }
}
</style>
