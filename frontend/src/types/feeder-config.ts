import type { IScheduleItem } from "./schedule-item";

export interface IFeederConfig {
  isFTPEnabled: boolean,
  isLEDEnabled: boolean,
  schedule: IScheduleItem[],
}
