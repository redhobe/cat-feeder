import type { IFeederConfig } from "@/types/feeder-config";
import { ref } from "vue";

export const useConfigApi = () => {
  const isLoading = ref<boolean>(false);

  async function getConfig(): Promise<IFeederConfig> {
    try {
      isLoading.value = true;
      const response = await fetch("http://192.168.1.97/config");
      return (await response.json()) as IFeederConfig;
    } catch (error) {
      alert(error);
      return {
        isFTPEnabled: false,
        isLEDEnabled: false,
        schedule: [],
      };
    } finally {
      isLoading.value = false;
    }
  }

  async function saveConfig(config: IFeederConfig) {
    try {
      isLoading.value = true;
      await fetch("http://192.168.1.97/config?config=" + JSON.stringify(config), {
        method: "PUT",
      });
      alert("Расписание успешно обновлено");
    } catch (error) {
      alert(`Ошибка при сохранении расписания: ${error}`);
    } finally {
      isLoading.value = false;
    }
  }

  return {
    isLoading,
    getConfig,
    saveConfig
  }
}
